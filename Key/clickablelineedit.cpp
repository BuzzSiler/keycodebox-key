#include "clickablelineedit.h"
#include <QDebug>

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
    Q_UNUSED(event);
    emit clicked();
}
