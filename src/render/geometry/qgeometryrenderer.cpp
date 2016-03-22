/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
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

#include "qgeometryrenderer.h"
#include "qgeometryrenderer_p.h"

#include <private/qcomponent_p.h>
#include <Qt3DCore/qscenepropertychange.h>

QT_BEGIN_NAMESPACE

using namespace Qt3DCore;

namespace Qt3DRender {

QGeometryRendererPrivate::QGeometryRendererPrivate()
    : QComponentPrivate()
    , m_instanceCount(1)
    , m_vertexCount(0)
    , m_indexOffset(0)
    , m_firstInstance(0)
    , m_restartIndexValue(-1)
    , m_verticesPerPatch(0)
    , m_primitiveRestart(false)
    , m_geometry(Q_NULLPTR)
    , m_primitiveType(QGeometryRenderer::Triangles)
{
}

/*!
 * \qmltype GeometryRenderer
 * \instantiates Qt3DRender::QGeometryRenderer
 * \inqmlmodule Qt3D.Render
 */

/*!
 * \qmlproperty int GeometryRenderer::instanceCount
 *
 * Holds the instance count.
 */

/*!
 * \qmlproperty int GeometryRenderer::vertexCount
 *
 * Holds the vertex count.
 */

/*!
 * \qmlproperty int GeometryRenderer::indexOffset
 *
 * Holds the base vertex.
 */

/*!
 * \qmlproperty int GeometryRenderer::firstInstance
 *
 * Holds the base instance.
 */

/*!
 * \qmlproperty int GeometryRenderer::restartIndex
 *
 * Holds the restart index.
 */

/*!
 * \qmlproperty int GeometryRenderer::verticesPerPatch
 *
 * Holds vertices per patch.
 */

/*!
 * \qmlproperty bool GeometryRenderer::primitiveRestart
 *
 * Holds the primitive restart flag.
 */

/*!
 * \qmlproperty Geometry GeometryRenderer::geometry
 *
 * Holds the geometry.
 */

/*!
 * \qmlproperty QGeometryRenderer::PrimitiveType GeometryRenderer::primitiveType
 *
 * Holds the primitive type.
 */

/*!
 * \class Qt3DRender::QGeometryRenderer
 * \inmodule Qt3DRender
 *
 * \inherits Qt3DCore::QComponent
 *
 */

/*!
 * \enum QGeometryRenderer::PrimitiveType
 *
 * The type of the primitive.
 *
 * \value Points
 * \value Lines
 * \value LineLoop
 * \value LineStrip
 * \value Triangles
 * \value TriangleStrip
 * \value TriangleFan
 * \value LinesAdjacency
 * \value TrianglesAdjacency
 * \value LineStripAdjacency
 * \value TriangleStripAdjacency
 * \value Patches
 */

/*!
 * Constructs a new QGeometryRenderer with \a parent.
 */
QGeometryRenderer::QGeometryRenderer(QNode *parent)
    : QComponent(*new QGeometryRendererPrivate(), parent)
{
}

/*!
 * Destroys this geometry renderer.
 */
QGeometryRenderer::~QGeometryRenderer()
{
    QComponent::cleanup();
}

/*!
 * \internal
 */
QGeometryRenderer::QGeometryRenderer(QGeometryRendererPrivate &dd, QNode *parent)
    : QComponent(dd, parent)
{
}

/*!
 * \property QGeometryRenderer::instanceCount
 *
 * Holds the instance count.
 */
int QGeometryRenderer::instanceCount() const
{
    Q_D(const QGeometryRenderer);
    return d->m_instanceCount;
}

/*!
 * \property QGeometryRenderer::vertexCount
 *
 * Holds the primitive count.
 */
int QGeometryRenderer::vertexCount() const
{
    Q_D(const QGeometryRenderer);
    return d->m_vertexCount;
}

/*!
 * \property QGeometryRenderer::indexOffset
 *
 * Holds the base vertex.
 */
int QGeometryRenderer::indexOffset() const
{
    Q_D(const QGeometryRenderer);
    return d->m_indexOffset;
}

/*!
 * \property QGeometryRenderer::firstInstance
 *
 * Holds the base instance.
 */
int QGeometryRenderer::firstInstance() const
{
    Q_D(const QGeometryRenderer);
    return d->m_firstInstance;
}

/*!
 * \property QGeometryRenderer::restartIndex
 *
 * Holds the restart index.
 */
int QGeometryRenderer::restartIndexValue() const
{
    Q_D(const QGeometryRenderer);
    return d->m_restartIndexValue;
}

/*!
 * \property QGeometry::verticesPerPatch
 *
 * Holds vertices per patch.
 */
int QGeometryRenderer::verticesPerPatch() const
{
    Q_D(const QGeometryRenderer);
    return d->m_verticesPerPatch;
}

/*!
 * \property QGeometryRenderer::primitiveRestart
 *
 * Holds the primitive restart flag.
 */
bool QGeometryRenderer::primitiveRestartEnabled() const
{
    Q_D(const QGeometryRenderer);
    return d->m_primitiveRestart;
}

/*!
 * \property QGeometryRenderer::geometry
 *
 * Holds the geometry.
 */
QGeometry *QGeometryRenderer::geometry() const
{
    Q_D(const QGeometryRenderer);
    return d->m_geometry;
}

/*!
 * \property QGeometryRenderer::primitiveType
 *
 * Holds the primitive type.
 */
QGeometryRenderer::PrimitiveType QGeometryRenderer::primitiveType() const
{
    Q_D(const QGeometryRenderer);
    return d->m_primitiveType;
}

/*!
 * \return the geometry functor.
 */
QGeometryFactoryPtr QGeometryRenderer::geometryFactory() const
{
    Q_D(const QGeometryRenderer);
    return d->m_geometryFactory;
}

void QGeometryRenderer::setInstanceCount(int instanceCount)
{
    Q_D(QGeometryRenderer);
    if (d->m_instanceCount == instanceCount)
        return;

    d->m_instanceCount = instanceCount;
    emit instanceCountChanged(instanceCount);
}

void QGeometryRenderer::setVertexCount(int vertexCount)
{
    Q_D(QGeometryRenderer);
    if (d->m_vertexCount == vertexCount)
        return;

    d->m_vertexCount = vertexCount;
    emit vertexCountChanged(vertexCount);
}

void QGeometryRenderer::setIndexOffset(int indexOffset)
{
    Q_D(QGeometryRenderer);
    if (d->m_indexOffset == indexOffset)
        return;

    d->m_indexOffset = indexOffset;
    emit indexOffsetChanged(indexOffset);
}

void QGeometryRenderer::setFirstInstance(int firstInstance)
{
    Q_D(QGeometryRenderer);
    if (d->m_firstInstance == firstInstance)
        return;

    d->m_firstInstance = firstInstance;
    emit firstInstanceChanged(firstInstance);
}

void QGeometryRenderer::setRestartIndexValue(int index)
{
    Q_D(QGeometryRenderer);
    if (index == d->m_restartIndexValue)
        return;

    d->m_restartIndexValue = index;
    emit restartIndexValueChanged(index);
}

void QGeometryRenderer::setVerticesPerPatch(int verticesPerPatch)
{
    Q_D(QGeometryRenderer);
    if (d->m_verticesPerPatch != verticesPerPatch) {
        d->m_verticesPerPatch = verticesPerPatch;
        emit verticesPerPatchChanged(verticesPerPatch);
    }
}

void QGeometryRenderer::setPrimitiveRestartEnabled(bool enabled)
{
    Q_D(QGeometryRenderer);
    if (enabled == d->m_primitiveRestart)
        return;

    d->m_primitiveRestart = enabled;
    emit primitiveRestartEnabledChanged(enabled);
}

void QGeometryRenderer::setGeometry(QGeometry *geometry)
{
    Q_D(QGeometryRenderer);
    if (d->m_geometry == geometry)
        return;

    if (d->m_geometry && d->m_changeArbiter) {
        QScenePropertyChangePtr change(new QScenePropertyChange(NodeRemoved, QSceneChange::Node, id()));
        change->setPropertyName("geometry");
        change->setValue(QVariant::fromValue(d->m_geometry->id()));
        d->notifyObservers(change);
    }

    if (geometry && !geometry->parent())
        geometry->setParent(this);

    d->m_geometry = geometry;
    const bool blocked = blockNotifications(true);
    emit geometryChanged(geometry);
    blockNotifications(blocked);

    if (d->m_geometry && d->m_changeArbiter) {
        QScenePropertyChangePtr change(new QScenePropertyChange(NodeAdded, QSceneChange::Node, id()));
        change->setPropertyName("geometry");
        change->setValue(QVariant::fromValue(d->m_geometry->id()));
        d->notifyObservers(change);
    }
}

void QGeometryRenderer::setPrimitiveType(QGeometryRenderer::PrimitiveType primitiveType)
{
    Q_D(QGeometryRenderer);
    if (d->m_primitiveType == primitiveType)
        return;

    d->m_primitiveType = primitiveType;
    emit primitiveTypeChanged(primitiveType);
}

/*!
 * Sets the geometry \a functor.
 */
void QGeometryRenderer::setGeometryFactory(const QGeometryFactoryPtr &factory)
{
    Q_D(QGeometryRenderer);
    if (factory && d->m_geometryFactory && *factory == *d->m_geometryFactory)
        return;
    d->m_geometryFactory = factory;
    if (d->m_changeArbiter != Q_NULLPTR) {
        QScenePropertyChangePtr change(new QScenePropertyChange(NodeUpdated, QSceneChange::Node, id()));
        change->setPropertyName("geometryFactory");
        change->setValue(QVariant::fromValue(d->m_geometryFactory));
        d->notifyObservers(change);
    }
}

/*!
 * \internal
 */
void QGeometryRenderer::copy(const QNode *ref)
{
    QComponent::copy(ref);
    const QGeometryRenderer *other = static_cast<const QGeometryRenderer *>(ref);
    d_func()->m_instanceCount = other->d_func()->m_instanceCount;
    d_func()->m_vertexCount = other->d_func()->m_vertexCount;
    d_func()->m_indexOffset = other->d_func()->m_indexOffset;
    d_func()->m_firstInstance = other->d_func()->m_firstInstance;
    d_func()->m_restartIndexValue = other->d_func()->m_restartIndexValue;
    d_func()->m_primitiveRestart = other->d_func()->m_primitiveRestart;
    d_func()->m_primitiveType = other->d_func()->m_primitiveType;
    d_func()->m_verticesPerPatch = other->d_func()->m_verticesPerPatch;
    d_func()->m_geometry = static_cast<QGeometry *>(QNode::clone(other->d_func()->m_geometry));
    d_func()->m_geometryFactory = other->d_func()->m_geometryFactory;
}

/*!
 * \internal
 */
void QGeometryRenderer::sceneChangeEvent(const Qt3DCore::QSceneChangePtr &e)
{
    Q_D(QGeometryRenderer);
    QScenePropertyChangePtr change = qSharedPointerCast<QScenePropertyChange>(e);
    if (change->type() == NodeUpdated && change->propertyName() == QByteArrayLiteral("geometry")) {
        QNodePtr nodePtr = change->value().value<QNodePtr>();
        QGeometry *backendGeometry = static_cast<QGeometry *>(nodePtr.data());
        QGeometry *frontendGeometry = static_cast<QGeometry *>(QNode::clone(backendGeometry));
        if (frontendGeometry != Q_NULLPTR)
            d->insertTree(frontendGeometry);
        setGeometry(frontendGeometry);
    }
}

Qt3DCore::QNodeCreatedChangeBasePtr QGeometryRenderer::createNodeCreationChange() const
{
    auto creationChange = Qt3DCore::QNodeCreatedChangePtr<QGeometryRendererData>::create(this);
    auto &data = creationChange->data;
    Q_D(const QGeometryRenderer);
    data.instanceCount = d->m_instanceCount;
    data.vertexCount = d->m_vertexCount;
    data.indexOffset = d->m_indexOffset;
    data.firstInstance = d->m_firstInstance;
    data.restartIndexValue = d->m_restartIndexValue;
    data.verticesPerPatch = d->m_verticesPerPatch;
    data.primitiveRestart = d->m_primitiveRestart;
    data.geometryId = qIdForNode(d->m_geometry);
    data.primitiveType = d->m_primitiveType;
    data.geometryFactory = d->m_geometryFactory;
    return creationChange;
}

} // namespace Qt3DRender

QT_END_NAMESPACE
