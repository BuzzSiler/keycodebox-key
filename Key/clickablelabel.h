#ifndef CCLICKABLELABEL_H
#define CCLICKABLELABEL_H

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QMouseEvent>

class CClickableLabel : public QLabel
{
    Q_OBJECT
    public:
        explicit CClickableLabel(QWidget* parent=0) : QLabel(parent) {}
        explicit CClickableLabel( const QString& text="", QWidget* parent=0 );
        ~CClickableLabel();

    signals:
        void clicked();

    protected:
        void mousePressEvent(QMouseEvent* event);

};

#endif // CCLICKABLELABEL_H
