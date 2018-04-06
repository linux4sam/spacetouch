/*
 * Copyright (C) 2018 Microchip Technology Inc.  All rights reserved.
 * Joshua Henderson <joshua.henderson@microchip.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "planemanager.h"
#include "graphicsplaneitem.h"
#include "tools.h"
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QProgressBar>
#include <QGraphicsProxyWidget>
#include <QTimer>
#include <QDesktopWidget>

#ifdef USE_PLANES
class GraphicsPixmapPlaneItem : public GraphicsPlaneItem
{
public:
    GraphicsPixmapPlaneItem(struct plane_data* plane, const QPixmap& pixmap)
        : GraphicsPlaneItem(plane, pixmap.rect())
    {
        draw(plane, pixmap.toImage());
        moveEvent(pos());
    }
};
#endif

class MyGraphicsView : public QGraphicsView
{
public:
    MyGraphicsView(QGraphicsScene *scene)
        : QGraphicsView(scene),
          m_move(false)
    {
        setAttribute(Qt::WA_NoSystemBackground);
        setViewportUpdateMode(ViewportUpdateMode::SmartViewportUpdate);

#ifdef USE_PLANES
        if (!m_planes.load("spacetouch.screen"))
        {
            QMessageBox::critical(0, "Failed to Setup Planes",
                                  "This demo requires a version of Qt that provides access to the DRI file descriptor,"
                                  " a valid planes screen.config file, and using the linuxfb backend with the env var "
                                  "QT_QPA_FB_DRM set.\n");
            qFatal("Failure setting up planes.");
        }

        m_plane1 = new GraphicsPixmapPlaneItem(m_planes.get("overlay0"), QPixmap(":/media/plane1.png").scaledToWidth(scene->width()));
        m_plane2 = new GraphicsPixmapPlaneItem(m_planes.get("overlay1"), QPixmap(":/media/plane2.png").scaledToWidth(scene->width()));
        m_plane3 = new GraphicsPixmapPlaneItem(m_planes.get("overlay2"), QPixmap(":/media/plane3.png").scaledToWidth(scene->width()));
#else
        m_plane1 = new QGraphicsPixmapItem(QPixmap(":/media/plane1.png"));
        m_plane2 = new QGraphicsPixmapItem(QPixmap(":/media/plane2.png"));
        m_plane3 = new QGraphicsPixmapItem(QPixmap(":/media/plane3.png"));
#endif
        scene->addItem(m_plane1);
        scene->addItem(m_plane2);
        scene->addItem(m_plane3);
    }

    void paintEvent(QPaintEvent * event)
    {
        qDebug() << "GraphicsPlaneView::paintEvent " << event->region().boundingRect();

        QGraphicsView::paintEvent(event);
    }

    void mousePressEvent(QMouseEvent *event)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            m_offset = event->pos();
            m_pos1 = m_plane1->pos();
            m_pos2 = m_plane2->pos();
            m_pos3 = m_plane3->pos();
            m_move = true;
        }

        QGraphicsView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent *event)
    {
        if (m_move)
        {
            if(event->buttons() & Qt::LeftButton)
            {
                QPointF p = event->pos() - m_offset;

                QPointF point1 = m_pos1 + (p * 0.1);
                QPointF point2 = m_pos2 + (p * 0.3);
                QPointF point3 = m_pos3 + (p * 0.5);

                QRectF bounds(QPointF(rect().topLeft().x() - rect().width() / 2,
                                      rect().topLeft().y() - rect().height() / 2),
                              rect().size() * 2);

                if (bounds.contains(QRectF(point1, rect().size())) &&
                        bounds.contains(QRectF(point2, rect().size())) &&
                        bounds.contains(QRectF(point3, rect().size())))
                {
                    m_plane1->setPos(point1);
                    m_plane2->setPos(point2);
                    m_plane3->setPos(point3);
                }
            }
        }

        QGraphicsView::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event)
    {
        m_move = false;

        QGraphicsView::mouseReleaseEvent(event);
    }

    void keyPressEvent(QKeyEvent* k)
    {
        if(k->key() == 48){
            QApplication::instance()->exit();
        }
    }

private:
#ifdef USE_PLANES
    PlaneManager m_planes;
    GraphicsPlaneItem* m_plane1;
    GraphicsPlaneItem* m_plane2;
    GraphicsPlaneItem* m_plane3;
#else
    QGraphicsPixmapItem* m_plane1;
    QGraphicsPixmapItem* m_plane2;
    QGraphicsPixmapItem* m_plane3;
#endif
    QPoint m_offset;
    QPointF m_pos1;
    QPointF m_pos2;
    QPointF m_pos3;
    bool m_move;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QRect screen = QApplication::desktop()->screenGeometry();

    QGraphicsScene scene;

    /*
     * Create scene items.  Some are standard Qt objects, others are custom ones that
     * translate to hardware plane usage behind the scenes.
     */

    QGraphicsPixmapItem* logo = new QGraphicsPixmapItem(QPixmap(":/media/logo.png"));
    logo->setPos(10, 10);
    scene.addItem(logo);

    QProgressBar* progress = new QProgressBar();
    progress->setOrientation(Qt::Horizontal);
    progress->setRange(0, 100);
    progress->setTextVisible(true);
    progress->setAlignment(Qt::AlignCenter);
    progress->setFormat("CPU: %p%");
    progress->setValue(0);
    QPalette p = progress->palette();
    p.setColor(QPalette::Foreground, Qt::white);
    p.setColor(QPalette::Highlight, Qt::red);
    p.setBrush(QPalette::Background, Qt::transparent);
    progress->setPalette(p);
    progress->setMaximumWidth(200);
    QGraphicsProxyWidget *proxy = scene.addWidget(progress);
    proxy->setPos(screen.width() - 200 - 10, 10);

    /*
     * Setup the view.
     */

    MyGraphicsView view(&scene);
    view.setStyleSheet("QGraphicsView { border-style: none; }");
    QPixmap background(":/media/plane0.png");
    view.setBackgroundBrush(background.scaled(screen.width(),
                                              screen.height(),
                                              Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation));
    view.setCacheMode(QGraphicsView::CacheBackground);
    view.resize(screen.width(), screen.height());
    view.setSceneRect(0, 0, screen.width(), screen.height());
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.show();

    /*
     * Update the progress bar independently.
     */

    Tools tools;
    QTimer cpuTimer;
    QObject::connect(&cpuTimer, &QTimer::timeout, [&tools,&progress]() {
        tools.updateCpuUsage();
        progress->setValue(tools.cpu_usage[0]);
    });
    cpuTimer.start(1000);

    return app.exec();
}
