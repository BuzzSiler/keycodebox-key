#include "autogeneratecontrolwidget.h"
#include "ui_autogeneratecontrolwidget.h"

#include "kcbcommon.h"

AutoGenerateControlWidget::AutoGenerateControlWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoGenerateControlWidget)
{
    ui->setupUi(this);

    ui->twGeneratedCodes->horizontalHeader()->hide();
    ui->twGeneratedCodes->verticalHeader()->hide();
    qsrand(54935220349342);
}

AutoGenerateControlWidget::~AutoGenerateControlWidget()
{
    delete ui;
}

void AutoGenerateControlWidget::on_cbSelectCode_currentIndexChanged(const QString &element)
{
    KCB_DEBUG_TRACE("Selected Code" << element);
}

void AutoGenerateControlWidget::on_pbGenerateCodes_clicked()
{
    KCB_DEBUG_TRACE("Generating Auto Codes");
    KCB_DEBUG_TRACE("\tSelect Code" << ui->cbSelectCode->currentText());
    KCB_DEBUG_TRACE("\tNum Digits" << ui->sbNumDigits->value());
    KCB_DEBUG_TRACE("\tGen Time" << ui->edGenerateTime->time());
    KCB_DEBUG_TRACE("\tGen Freq" << ui->cbGenerateFreq->currentText());
    KCB_DEBUG_TRACE("\tCode Block" << ui->cbCodeBlock->currentText());
    KCB_DEBUG_TRACE("\tCode Repeat" << ui->cbCodeRepeat->currentText());

    ui->twGeneratedCodes->clear();

    int cols = ui->twGeneratedCodes->columnCount();
    int rows = ui->cbCodeBlock->currentText().toInt() / cols;

    for (int col = 0; col < cols; ++col)
    {
        for (int row = 0; row < rows; ++row)
        {
            int rand = qrand();
            QString rand_str = QString::number(rand);
            QString value = rand_str.right(ui->sbNumDigits->value());
            QTableWidgetItem *newItem = new QTableWidgetItem(tr("%2").arg(value));
            ui->twGeneratedCodes->setItem(row, col, newItem);
        }
    }

}

void AutoGenerateControlWidget::on_pbApplyCodes_clicked()
{
    KCB_DEBUG_TRACE("Generating Auto Codes");
    KCB_DEBUG_TRACE("\tSelect Code" << ui->cbSelectCode->currentText());
    KCB_DEBUG_TRACE("\tNum Digits" << ui->sbNumDigits->value());
    KCB_DEBUG_TRACE("\tGen Time" << ui->edGenerateTime->time());
    KCB_DEBUG_TRACE("\tGen Freq" << ui->cbGenerateFreq->currentText());
    KCB_DEBUG_TRACE("\tCode Block" << ui->cbCodeBlock->currentText());
    KCB_DEBUG_TRACE("\tCode Repeat" << ui->cbCodeRepeat->currentText());
}
