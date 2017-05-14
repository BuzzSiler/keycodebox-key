#ifndef CMENUCHECKBOX_H
#define CMENUCHECKBOX_H

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>

class CMenuCheckBox : public QCheckBox
{
public:
    CMenuCheckBox(QWidget* pParent=0) : QCheckBox(pParent) {}
    CMenuCheckBox(const QString& text, QWidget* pParent = 0) : QCheckBox(text, pParent) {}

protected:
    virtual void mousePressEvent(QMouseEvent *ev) {
        QMenu MyMenu(this);
        MyMenu.addActions(this->actions());
        MyMenu.exec(ev->globalPos());
    }
};

#endif // CMENUCHECKBOX_H
