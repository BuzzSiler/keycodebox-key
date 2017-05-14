#include "clickablelineedit.h"

CClickableLineEdit::CClickableLineEdit(const QString& text, QWidget* parent)
    : QLineEdit(parent)
{
    setText(text);
}

CClickableLineEdit::~CClickableLineEdit()
{
}

void CClickableLineEdit::mousePressEvent(QMouseEvent* event)
{
    emit clicked();
}
