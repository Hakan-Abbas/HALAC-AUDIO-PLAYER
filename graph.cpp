#include "graph.h"

graph::graph(QWidget* parent) : QGraphicsView(parent)
{

}

void graph::mouseMoveEvent(QMouseEvent *event)
{
    QPointF scenePoint = mapToScene(event->pos());
    x = scenePoint.x();
    //qDebug () << "Move" << scenePoint.x();
    if(event->buttons() & Qt::LeftButton) emit mousePos();
}

void graph::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed();
    //if(event->buttons() & Qt::RightButton) qDebug() <<"press";
}

void graph::leaveEvent(QEvent *event)
{
    emit mouseLeave();
    //qDebug () << "Leave";
}
