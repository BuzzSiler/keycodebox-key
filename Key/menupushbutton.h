#ifndef CMENUPUSHBUTTON_H
#define CMENUPUSHBUTTON_H

#include <QDebug>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>

class CMenuPushButton : public QPushButton
{
public:
    CMenuPushButton(QWidget* pParent=0) : QPushButton(pParent) {}
    CMenuPushButton(const QString& text, QWidget* pParent = 0) : QPushButton(text, pParent) {}
    CMenuPushButton(const QIcon& icon, const QString &text, QWidget* pParent = 0) : QPushButton(icon, text, pParent) {}

protected:
    virtual void mouseReleaseEvent(QMouseEvent *ev) {
        qDebug() << "CMenuPushButton::mouseReleaseEvent()";
        QMenu MyMenu(this);
        MyMenu.addActions(this->actions());
        MyMenu.exec(ev->globalPos());
    }};

#endif // CMENUPUSHBUTTON_H
