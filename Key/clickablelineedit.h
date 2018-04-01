#ifndef CCLICKABLELINEEDIT_H
#define CCLICKABLELINEEDIT_H

#include <QObject>
#include <QLineEdit>

class QMouseEvent;

class CClickableLineEdit : public QLineEdit
{
    Q_OBJECT
    public:
        explicit CClickableLineEdit(QWidget* parent=0) : QLineEdit(parent) {}
        explicit CClickableLineEdit( const QString& text="", QWidget* parent=0 );
        ~CClickableLineEdit();

    signals:
        void clicked();

    protected:
        void mousePressEvent(QMouseEvent* event);
};

#endif // CCLICKABLELINEEDIT_H
