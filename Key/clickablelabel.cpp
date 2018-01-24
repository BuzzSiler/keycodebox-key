#include "clickablelabel.h"

CClickableLabel::CClickableLabel(const QString& text, QWidget* parent)
    : QLabel(parent)
{
    setText(text);
}

CClickableLabel::~CClickableLabel()
{
}

void CClickableLabel::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    emit clicked();
}
