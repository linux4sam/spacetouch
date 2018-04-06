/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 * Joshua Henderson <joshua.henderson@microchip.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "graphicsplaneitem.h"
#include <planes/plane.h>
#include <QPainter>
#include <QDebug>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

GraphicsPlaneItem::GraphicsPlaneItem(struct plane_data* plane, const QRectF& bounding)
    : m_bounding(bounding),
      m_plane(plane)
{
    if (!plane)
        qFatal("invalid plane pointer");

    /*
     * This is not a solid solution, but this prevents painting in most cases.  We don't need
     * to paint a plane unless the plane contents itself changes.  This means, we don't need
     * to paint when the item is moved, scaled, etc.
     */
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    /*
     * QGraphicsItem::ItemSendsGeometryChanges is how we get ItemPositionChange,
     * ItemPositionHasChanged, ItemMatrixChange, ItemTransformChange, ItemTransformHasChanged,
     * ItemRotationChange, ItemRotationHasChanged, ItemScaleChange, ItemScaleHasChanged,
     * ItemTransformOriginPointChange, and ItemTransformOriginPointHasChanged to itemChange().
     *
     * QGraphicsItem::ItemHasNoContents is the magic that prevents paint calls from the view/scene.
     */
    setFlags(QGraphicsItem::ItemSendsGeometryChanges |
             QGraphicsItem::ItemClipsToShape |
             QGraphicsItem::ItemHasNoContents);

    moveEvent(pos());
}

QVariant GraphicsPlaneItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    qDebug() << "GraphicsPlaneItem::itemChange " << change;

    if (change == GraphicsItemChange::ItemPositionChange)
    {
        moveEvent(value.toPoint());
        return value;
    }
    else if (change == GraphicsItemChange::ItemPositionHasChanged)
    {
        moveEvent(value.toPoint());
        return value;
    }
    else if (change == GraphicsItemChange::ItemScaleChange)
    {
        // TODO: validate scale
    }
    else if (change == GraphicsItemChange::ItemScaleHasChanged)
    {
        qDebug() << "scale " << value.toFloat();
        plane_set_scale(m_plane, value.toFloat());
        plane_apply(m_plane);
    }

    return QGraphicsItem::itemChange(change, value);
}

void GraphicsPlaneItem::moveEvent(const QPointF& point)
{
    qDebug() << "GraphicsPlaneItem::moveEvent " << point;

    plane_set_pos(m_plane, point.x(), point.y());
    plane_apply(m_plane);
}

void GraphicsPlaneItem::draw(struct plane_data* plane, QImage image, bool horizontal, bool vertical, bool scale)
{
    if ((int)plane_width(plane) != image.width() || (int)plane_height(plane) != image.height())
        plane_fb_reallocate(plane, image.width(), image.height(), plane_format(plane));

    plane_fb_map(plane);

    QImage fb(static_cast<uchar*>(plane->buf),
              plane_width(plane), plane_height(plane),
              QImage::Format_ARGB32_Premultiplied);

    QPainter painter(&fb);
    painter.setTransform(transform());
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    QImage transformedImage(image);

    if (scale)
    {
        QSize imageSize = image.size();
        imageSize.scale(QSize(plane_width(plane), plane_height(plane)), Qt::KeepAspectRatio);

        transformedImage = image.scaled(imageSize,
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
    }

    if (horizontal || vertical)
    {
        transformedImage = transformedImage.mirrored(horizontal, vertical);
    }

    painter.drawImage(QPoint(0,0), transformedImage);
    painter.end();
}
