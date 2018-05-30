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

    private slots:

        void on_cbSelectCode_currentIndexChanged(const QString &element);

        void on_pbGenerateCodes_clicked();

        void on_pbApplyCodes_clicked();

    private:
        Ui::AutoGenerateControlWidget *ui;
};

#endif // AUTOGENERATECONTROLWIDGET_H
