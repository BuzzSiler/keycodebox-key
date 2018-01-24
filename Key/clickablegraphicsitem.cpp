#include "clickablegraphicsitem.h"

void CClickableGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event = event;
}


void CClickableGraphicsView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Custom view clicked.";
    QGraphicsView::mousePressEvent(event);
    clickedFunc();
}
