#include "clickablelineedit.h"
#include <QDebug>
#include <QMouseEvent>

CClickableLineEdit::CClickableLineEdit(const QString& text, QWidget* parent) :
    QLineEdit(parent)
{
    setText(text);
}

CClickableLineEdit::~CClickableLineEdit()
{
}

void CClickableLineEdit::mousePressEvent(QMouseEvent* event)
{
    qDebug() << event->type();
    //QEvent::MouseButtonPress, QEvent::MouseButtonRelease, QEvent::MouseButtonDblClick, or QEvent::MouseMove
    emit clicked();
}
