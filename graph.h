#ifndef GRAPH_H
#define GRAPH_H

#include <QGraphicsView>
#include <QMouseEvent>
#include <QEvent>

class graph : public QGraphicsView
{
    Q_OBJECT
public:
    explicit graph(QWidget *parent = 0);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void leaveEvent(QEvent *);
    quint32 x,y;

signals:
    void mousePressed();
    void mousePos();
    void mouseLeave();
};

#endif // GRAPH_H
