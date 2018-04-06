/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 * Joshua Henderson <joshua.henderson@microchip.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef GRAPHICSPLANEITEM_H
#define GRAPHICSPLANEITEM_H

#include <QGraphicsObject>
#include <QDebug>
#include "planemanager.h"
#include <QGraphicsView>

/**
 * @brief The GraphicsPlaneItem class
 *
 * A QGraphicsObject that translates functions away from native Qt operations into hardware
 * planes functions using libplanes.
 */
class GraphicsPlaneItem : public QGraphicsObject
{
public:

    GraphicsPlaneItem(struct plane_data* plane, const QRectF& bounding);

    virtual QRectF boundingRect() const override
    {
        return m_bounding;
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override
    {
        Q_UNUSED(painter);
        Q_UNUSED(option);
        Q_UNUSED(widget);
    }

    /**
     * @brief update
     * Custom version of Qt required to make this virtual.
     */
    virtual void update(const QRectF &rect = QRectF())
    {
        qDebug() << "GraphicsPlaneItem::update aborted";

        Q_UNUSED(rect);
    }

    virtual ~GraphicsPlaneItem()
    {}

protected:

    virtual void moveEvent(const QPointF& point);

    /**
     * @brief draw
     *
     * A special drawing function that draws an image directly to a plane.
     *
     * @param plane
     * @param image
     * @param horizontal
     * @param vertical
     */
    void draw(struct plane_data* plane, QImage image, bool horizontal = false, bool vertical = false, bool scale = true);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    QRectF m_bounding;
    struct plane_data* m_plane;
};

#endif // GRAPHICSPLANEITEM_H

