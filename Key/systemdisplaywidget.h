#ifndef SYSTEMDISPLAYWIDGET_H
#define SYSTEMDISPLAYWIDGET_H

#include <QWidget>

namespace Ui {
    class SystemDisplayWidget;
}

class SystemDisplayWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SystemDisplayWidget(QWidget *parent = 0);
        ~SystemDisplayWidget();

    private:
        Ui::SystemDisplayWidget *ui;
};

#endif // SYSTEMDISPLAYWIDGET_H
