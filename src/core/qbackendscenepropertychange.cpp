/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qbackendscenepropertychange.h"
#include "qbackendscenepropertychange_p.h"
#include <Qt3DCore/qbackendnode.h>
#include <Qt3DCore/private/qbackendnode_p.h>

QT_BEGIN_NAMESPACE

namespace Qt3DCore {

/*!
 * \class Qt3DCore::QBackendScenePropertyChange
 * \inherits Qt3DCore::QScenePropertyChange
 * \inmodule Qt3DCore
 *
 * TODO
 */

QBackendScenePropertyChangePrivate::QBackendScenePropertyChangePrivate()
    : QNodePropertyChangeBasePrivate()
{
}

QBackendScenePropertyChangePrivate::~QBackendScenePropertyChangePrivate()
{
}

QBackendScenePropertyChange::QBackendScenePropertyChange(QNodeId subjectId, QSceneChange::Priority priority)
    : QNodePropertyChangeBase(*new QBackendScenePropertyChangePrivate, NodeUpdated, Observable, subjectId, priority)
{
}

QBackendScenePropertyChange::~QBackendScenePropertyChange()
{
}

/*!
 * \return property value.
 */
QVariant QBackendScenePropertyChange::value() const
{
    Q_D(const QBackendScenePropertyChange);
    return d->m_value;
}

/*!
 * Set the property change \a value.
 */
void QBackendScenePropertyChange::setValue(const QVariant &value)
{
    Q_D(QBackendScenePropertyChange);
    d->m_value = value;
}

// TO DO get rid off setTargetNode, use the subject instead ??
/*!
 * Sets the target node of the backend scene property change to \a id.
 */
void QBackendScenePropertyChange::setTargetNode(QNodeId id)
{
    Q_D(QBackendScenePropertyChange);
    d->m_targetUuid = id;
}

/*!
 * \return the target node id of the backend scene property change.
 */
QNodeId QBackendScenePropertyChange::targetNode() const
{
    Q_D(const QBackendScenePropertyChange);
    return d->m_targetUuid;
}

/*!
 * \typedef Qt3DCore::QBackendScenePropertyChangePtr
 * \relates Qt3DCore::QBackendScenePropertyChange
 *
 * A shared pointer for QBackendScenePropertyChange.
 */

/*!
 * \internal
 */
QBackendScenePropertyChange::QBackendScenePropertyChange(QBackendScenePropertyChangePrivate &dd)
    : QNodePropertyChangeBase(dd)
{
}

/*!
 * \internal
 */
QBackendScenePropertyChange::QBackendScenePropertyChange(QBackendScenePropertyChangePrivate &dd,
                                                         QNodeId subjectId, QSceneChange::Priority priority)
    : QNodePropertyChangeBase(dd, NodeUpdated, Observable, subjectId, priority)
{
}

} // Qt3D

QT_END_NAMESPACE
