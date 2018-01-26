#include "clickablegraphicsitem.h"

void CClickableGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
}


void CClickableGraphicsView::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "Custom view clicked.";
    QGraphicsView::mousePressEvent(event);
    clickedFunc();
}
