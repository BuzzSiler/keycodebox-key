#include "clickablegraphicsitem.h"

void CClickableGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{

}


void CClickableGraphicsView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Custom view clicked.";
    QGraphicsView::mousePressEvent(event);
    clickedFunc();
}
