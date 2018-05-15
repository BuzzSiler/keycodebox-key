#ifndef AUTOGENERATECONTROLWIDGET_H
#define AUTOGENERATECONTROLWIDGET_H

#include <QWidget>

namespace Ui {
    class AutoGenerateControlWidget;
}

class AutoGenerateControlWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit AutoGenerateControlWidget(QWidget *parent = 0);
        ~AutoGenerateControlWidget();

    private:
        Ui::AutoGenerateControlWidget *ui;
};

#endif // AUTOGENERATECONTROLWIDGET_H
