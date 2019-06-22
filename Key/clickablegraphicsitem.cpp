#include "clickablegraphicsitem.h"
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "kcbcommon.h"

void CClickableGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event = event;
}


void CClickableGraphicsView::mousePressEvent(QMouseEvent *event)
{
    // KCB_DEBUG_ENTRY;
    QGraphicsView::mousePressEvent(event);
    clickedFunc();
    // KCB_DEBUG_EXIT;
}
