/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qchangearbiter_p.h"
#include "qcomponent.h"
#include "qjobmanagerinterface.h"

#include "corelogging.h"
#include "qsceneobserverinterface.h"
#include "qsceneinterface.h"
#include <QMutexLocker>
#include <QReadLocker>
#include <QThread>
#include <QWriteLocker>
#include <private/qpostman_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3D {

/*!
 * \class QChangeArbiter
 * \namespace Qt3D
 * \since 5.4
 *
 * \brief Act as a messages router between observables and observers.
 * Observables can be of two types : QNode observables and QObservableInterfaces.
 * QNode observables notifications are sent from the frontend QNode to the backend observers.
 * QObservableInterface notifications are sent from backend to backend observers and/or backend to  QPostman,
 * the QPostman to deliver the messages to the frontend QNode.
 *
 * QNode observables are registered automatically. However QObservableInterface have to be registered manually
 * by providing the QUuid of the frontend QNode that observables is mapped to.
 *
 * Observers can be registered to receive messages from a QObservableInterface/QNode observable by providing a QNode QUuid.
 * When a notification from a QObservableInterface is received, it is then sent to all observers observing the
 * QNode QUuid as well as the QPostman to update the frontend QNode.
 */


QChangeArbiter::QChangeArbiter(QObject *parent)
    : QObject(parent)
    , m_mutex(QMutex::Recursive)
    , m_jobManager(Q_NULLPTR)
    , m_postman(Q_NULLPTR)
    , m_scene(Q_NULLPTR)
{
    // The QMutex has to be recursive to handle the case where :
    // 1) SyncChanges is called, mutex is locked
    // 2) Changes are distributed
    // 3) An observer decides to register a new observable upon receiving notification
    // 4) registerObserver locks the mutex once again -> we need recursion otherwise deadlock
    // 5) Mutex is unlocked - leaving registerObserver
    // 6) Mutex is unlocked - leaving SyncChanges
}

QChangeArbiter::~QChangeArbiter()
{
    if (m_jobManager != Q_NULLPTR)
        m_jobManager->waitForPerThreadFunction(QChangeArbiter::destroyThreadLocalChangeQueue, this);
    m_lockingChangeQueues.clear();
    m_changeQueues.clear();
}

void QChangeArbiter::initialize(QJobManagerInterface *jobManager)
{
    Q_CHECK_PTR(jobManager);
    m_jobManager = jobManager;

    // Init TLS for the change queues
    m_jobManager->waitForPerThreadFunction(QChangeArbiter::createThreadLocalChangeQueue, this);
}

void QChangeArbiter::distributeQueueChanges(ChangeQueue *changeQueue)
{
    for (int i = 0; i < changeQueue->size(); i++) {
        QSceneChangePtr change = changeQueue->takeFirst();
        // Lookup which observers care about the subject this change came from
        // and distribute the change to them
        if (change.isNull())
            continue;

        if (change->type() == NodeCreated) {
            Q_FOREACH (QSceneObserverInterface *observer, m_sceneObservers)
                observer->sceneNodeAdded(change);
        }
        else if (change->type() == NodeAboutToBeDeleted || change->type() == NodeDeleted) {
            Q_FOREACH (QSceneObserverInterface *observer, m_sceneObservers)
                observer->sceneNodeRemoved(change);
        }

        switch (change->observableType()) {

        case QSceneChange::Observable: {
            QObservableInterface *subject = change->subject().m_observable;
            QUuid nodeId = m_scene->nodeIdFromObservable(subject);
            if (m_nodeObservations.contains(nodeId)) {
                QObserverList &observers = m_nodeObservations[nodeId];
                Q_FOREACH (const QObserverPair&observer, observers) {
                    if ((change->type() & observer.first))
                        observer.second->sceneChangeEvent(change);
                }
                // Also send change to the postman
                m_postman->sceneChangeEvent(change);
            }
            break;
        }

        case QSceneChange::Node: {
            QNode *subject = change->subject().m_node;
            if (m_nodeObservations.contains(subject->uuid())) {
                QObserverList &observers = m_nodeObservations[subject->uuid()];
                Q_FOREACH (const QObserverPair&observer, observers) {
                    if ((change->type() & observer.first))
                        observer.second->sceneChangeEvent(change);
                }
            }
            break;
        }

        } // observableType switch
    }
}

QThreadStorage<QChangeArbiter::ChangeQueue *> *QChangeArbiter::tlsChangeQueue()
{
    return &(m_tlsChangeQueue);
}

void QChangeArbiter::appendChangeQueue(QChangeArbiter::ChangeQueue *queue)
{
    QMutexLocker locker(&m_mutex);
    m_changeQueues.append(queue);
}

void QChangeArbiter::removeChangeQueue(QChangeArbiter::ChangeQueue *queue)
{
    QMutexLocker locker(&m_mutex);
    m_changeQueues.removeOne(queue);
}

void QChangeArbiter::appendLockingChangeQueue(QChangeArbiter::ChangeQueue *queue)
{
    QMutexLocker locker(&m_mutex);
    m_lockingChangeQueues.append(queue);
}

void QChangeArbiter::syncChanges()
{
    QMutexLocker locker(&m_mutex);
    Q_FOREACH (QChangeArbiter::ChangeQueue *changeQueue, m_changeQueues)
        distributeQueueChanges(changeQueue);

    Q_FOREACH (ChangeQueue *changeQueue, m_lockingChangeQueues)
        distributeQueueChanges(changeQueue);
}

void QChangeArbiter::setScene(QSceneInterface *scene)
{
    m_scene = scene;
}

QObserverInterface *QChangeArbiter::postman() const
{
    return m_postman;
}

QSceneInterface *QChangeArbiter::scene() const
{
    return m_scene;
}

void QChangeArbiter::registerObserver(QObserverInterface *observer,
                                      const QUuid &nodeId,
                                      ChangeFlags changeFlags)
{
    QMutexLocker locker(&m_mutex);
    QObserverList &observerList = m_nodeObservations[nodeId];
    observerList.append(QObserverPair(changeFlags, observer));
}

// Called from the QAspectThread context, no need to lock
void QChangeArbiter::registerSceneObserver(QSceneObserverInterface *observer)
{
    if (!m_sceneObservers.contains(observer))
        m_sceneObservers << observer;
}

void QChangeArbiter::unregisterObserver(QObserverInterface *observer, const QUuid &nodeId)
{
    QMutexLocker locker(&m_mutex);
    if (m_nodeObservations.contains(nodeId)) {
        QObserverList &observers = m_nodeObservations[nodeId];
        for (int i = observers.count() - 1; i >= 0; i--) {
            if (observers[i].second == observer)
                observers.removeAt(i);
        }
    }
}

// Called from the QAspectThread context, no need to lock
void QChangeArbiter::unregisterSceneObserver(QSceneObserverInterface *observer)
{
    if (observer != Q_NULLPTR)
        m_sceneObservers.removeOne(observer);
}

void QChangeArbiter::sceneChangeEvent(const QSceneChangePtr &e)
{
    //    qCDebug(ChangeArbiter) << Q_FUNC_INFO << QThread::currentThread();

    // Add the change to the thread local storage queue - no locking required => yay!
    ChangeQueue *localChangeQueue = m_tlsChangeQueue.localData();
    localChangeQueue->append(e);

    //    qCDebug(ChangeArbiter) << "Change queue for thread" << QThread::currentThread() << "now contains" << localChangeQueue->count() << "items";
}

void QChangeArbiter::sceneChangeEventWithLock(const QSceneChangePtr &e)
{
    QMutexLocker locker(&m_mutex);
    sceneChangeEvent(e);
}

// Either we have the postman or we could make the QChangeArbiter agnostic to the postman
// but that would require adding it to every QObserverList in m_aspectObservations.
void QChangeArbiter::setPostman(QObserverInterface *postman)
{
    if (m_postman != postman) {
        // Unregister old postman here if needed
        m_postman = postman;
    }
}

void QChangeArbiter::createUnmanagedThreadLocalChangeQueue(void *changeArbiter)
{
    Q_CHECK_PTR(changeArbiter);

    QChangeArbiter *arbiter = static_cast<QChangeArbiter *>(changeArbiter);

    qCDebug(ChangeArbiter) << Q_FUNC_INFO << QThread::currentThread();
    if (!arbiter->tlsChangeQueue()->hasLocalData()) {
        ChangeQueue *localChangeQueue = new ChangeQueue;
        arbiter->tlsChangeQueue()->setLocalData(localChangeQueue);
        arbiter->appendLockingChangeQueue(localChangeQueue);
    }
}

void QChangeArbiter::destroyUnmanagedThreadLocalChangeQueue(void *changeArbiter)
{
    // TODO: Implement me!
    Q_UNUSED(changeArbiter);
}

void QChangeArbiter::createThreadLocalChangeQueue(void *changeArbiter)
{
    Q_CHECK_PTR(changeArbiter);

    QChangeArbiter *arbiter = static_cast<QChangeArbiter *>(changeArbiter);

    qCDebug(ChangeArbiter) << Q_FUNC_INFO << QThread::currentThread();
    if (!arbiter->tlsChangeQueue()->hasLocalData()) {
        ChangeQueue *localChangeQueue = new ChangeQueue;
        arbiter->tlsChangeQueue()->setLocalData(localChangeQueue);
        arbiter->appendChangeQueue(localChangeQueue);
    }
}

void QChangeArbiter::destroyThreadLocalChangeQueue(void *changeArbiter)
{
    // TODO: Implement me!
    Q_UNUSED(changeArbiter);
    QChangeArbiter *arbiter = static_cast<QChangeArbiter *>(changeArbiter);
    if (arbiter->tlsChangeQueue()->hasLocalData()) {
        ChangeQueue *localChangeQueue = arbiter->tlsChangeQueue()->localData();
        arbiter->removeChangeQueue(localChangeQueue);
        arbiter->tlsChangeQueue()->setLocalData(Q_NULLPTR);
    }
}

} // namespace Qt3D

QT_END_NAMESPACE
