#ifndef CCLICKABLEGRAPHICSITEM_H
#define CCLICKABLEGRAPHICSITEM_H


#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

#include "keycodeboxmain.h"

namespace Ui {
class MainWindow;
}

class CClickableGraphicsView : public QGraphicsView
{
public:
    CClickableGraphicsView(QWidget *parent = 0)
        : QGraphicsView(parent) {}

    CClickableGraphicsView(QGraphicsScene *scene, QWidget *parent = 0)
        : QGraphicsView(scene, parent) {}

    typedef void (*callback_func_ptr)();
    callback_func_ptr clickedFunc;

    void setClickedFunc(callback_func_ptr funcptr)
    {
        clickedFunc = funcptr;
    }

protected:
    void mousePressEvent(QMouseEvent *event);
};

class CClickableGraphicsItem : public QGraphicsPixmapItem
{

public:
    explicit CClickableGraphicsItem(QGraphicsItem *parent = 0)
        : QGraphicsPixmapItem(parent) {}
    explicit CClickableGraphicsItem(const QPixmap &pixmap, QGraphicsItem *parent = 0)
        : QGraphicsPixmapItem(pixmap, parent) {}

signals:
//    void clicked();

public slots:
    void	mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // CCLICKABLEGRAPHICSITEM_H
