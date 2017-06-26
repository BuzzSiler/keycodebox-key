#include "frmcodeedit.h"
#include "ui_frmcodeedit.h"
#include "tblcodes.h"
#include <QDebug>
#include <sqlite3.h>
#include <sqlite3ext.h>
#include <QtSql>
#include <string>
#include <QMessageBox>

CFrmCodeEdit::CFrmCodeEdit(QWidget *parent) :
    QDialog(parent), _pcurrentLineEdit(0),
    ui(new Ui::CFrmCodeEdit)
{
    ui->setupUi(this);
    CFrmCodeEdit::showFullScreen();
}

void CFrmCodeEdit::setMaxLocks(int nLocks)
{
    ui->spinLockNum->setMaximum(nLocks);
}

CFrmCodeEdit::~CFrmCodeEdit()
{
    delete ui;
    if(_pcurrentLineEdit) {
        delete _pcurrentLineEdit;
    }
}

void CFrmCodeEdit::setEditingRow(int nRow)
{
    _nRow = nRow;
    if(nRow == -1)
    {
        ui->chkFingerPrint1->setChecked(false);
        ui->edtAccessCode->setDisabled(false);
        ui->edtSecondCode->setDisabled(false);
    }
}

void CFrmCodeEdit::getValues(int *pId, int *pLockNum, QString *psAccessCode, QString *psSecondCode, QString *psDesc,
                             QDateTime *pdtStart, QDateTime *pdtEnd, bool *pFingerprint1, bool *pFingerprint2)
{
    *pId = _id;
    *pLockNum = ui->spinLockNum->value();
    *psAccessCode = ui->edtAccessCode->text();
    *psSecondCode = ui->edtSecondCode->text().trimmed();
    *psDesc = ui->edtDescription->text();
    *pdtStart = ui->dtStartAccess->dateTime();
    *pdtEnd = ui->dtEndAccess->dateTime();
    *pFingerprint1 = ui->chkFingerPrint1->isChecked();
    *pFingerprint2 = ui->chkFingerPrint2->isChecked();
}

void CFrmCodeEdit::setValues(int id, int nLockNum, QString sAccessCode, QString sSecondCode, QString sDesc,
                             QDateTime dtStart, QDateTime dtEnd, bool fingerprint1, bool fingerprint2,
                             bool askQuestions, std::string question1, std::string question2, std::string question3)
{
    qDebug() << "Editing Code ID:" << QVariant(id).toString() << " Lock:" << QVariant(nLockNum).toString() << " AccessCd:" << sAccessCode << " fingerprint: " << QString::number(fingerprint1) << " fingerprint1: " << QString::number(fingerprint2) << " askQuestions: " << QString::number(askQuestions) << " question1: " << QString::fromStdString(question1) << " question2: " << QString::fromStdString(question2) << " question3: " << QString::fromStdString(question3);

    _id = id;
    ui->codeId->setText(QVariant(_id).toString());
    ui->spinLockNum->setValue(nLockNum);
    ui->edtAccessCode->setText(sAccessCode);
    ui->edtSecondCode->setText(sSecondCode);
    ui->edtDescription->setText(sDesc);
    ui->dtStartAccess->setDateTime(dtStart);
    ui->dtEndAccess->setDateTime(dtEnd);

    if( fingerprint1 )
    {
        ui->chkFingerPrint1->setChecked(true);
        ui->edtAccessCode->setDisabled(true);
        ui->edtSecondCode->setDisabled(true);
    }
    if( fingerprint2)
    {
        ui->chkFingerPrint2->setChecked(true);
    }
    _askQuestions = askQuestions;
    _question1 = QString::fromStdString(question1);
    _question2 = QString::fromStdString(question2);
    _question3 = QString::fromStdString(question3);

    if( dtStart == (QDateTime(QDate(1990,1,1), QTime(0,0,0))) && dtEnd == (QDateTime(QDate(1990,1,1), QTime(0,0,0))))
    {
        ui->dtStartAccess->hide();
        ui->dtEndAccess->hide();
        ui->strStartAccess->show();
        ui->strEndAccess->show();
        ui->chkAllAccess->setChecked(true);
    } else {
        ui->dtStartAccess->show();
        ui->dtEndAccess->show();
        ui->strStartAccess->hide();
        ui->strEndAccess->hide();
        ui->chkAllAccess->setChecked(false);
    }

}

void CFrmCodeEdit::on_buttonBox_accepted()
{
    qDebug() << "on_buttonBox_accepted()";

    // if we have a 'set all locks' checkbox set,
    // spin and increment the spinLockNum from 1-END
    int i = 0;
    qDebug() << "ROW ID : " << QString::number(_id);
    qDebug() << "f1 isChecked: " << ui->chkFingerPrint1->isChecked();
    qDebug() << "f2 isChecked: " << ui->chkFingerPrint2->isChecked();

    qDebug() << "------------------------QUESTION 1: " << _question1;

    qDebug() << "------------------------QUESTION 2: " << _question2;

    qDebug() << "------------------------QUESTION 3: " << _question3;
    for(i=0; i<ui->spinLocksToCreate->value(); i++)
        emit(OnDoneSave(_nRow, _id, ui->spinLockNum->value(), ui->edtAccessCode->text(), ui->edtSecondCode->text(),
                        ui->edtDescription->text(), ui->dtStartAccess->dateTime(), ui->dtEndAccess->dateTime(),
                        ui->chkFingerPrint1->isChecked(), ui->chkFingerPrint2->isChecked(),0,
                        _question1, _question2, _question3));
    this->hide();
}

void CFrmCodeEdit::on_buttonBox_rejected()
{
    qDebug() << "on_buttonBox_rejected()";
    emit OnClose();
}

void CFrmCodeEdit::on_edtAccessCode_clicked()
{
    checkAndCreateCurrentLineEdit();

    _pcurrentLineEdit->setNumbersOnly();
    onStartEditLine(ui->edtAccessCode, "Code #1");
}

void CFrmCodeEdit::on_edtSecondCode_clicked()
{
    checkAndCreateCurrentLineEdit();

    _pcurrentLineEdit->setNumbersOnly();
    onStartEditLine(ui->edtSecondCode, "Code #2");
}

void CFrmCodeEdit::on_edtDescription_clicked()
{
    checkAndCreateCurrentLineEdit();

    _pcurrentLineEdit->clearInputMasks();
    onStartEditLine(ui->edtDescription, "Username");
}

void CFrmCodeEdit::checkAndCreateCurrentLineEdit()
{
    if(!_pcurrentLineEdit) {
        _pcurrentLineEdit = new CCurrentEdit();
        connect(this, SIGNAL(__OnCodes(QString)), _pcurrentLineEdit, SLOT(OnStringEntry(QString)));
    }
}

void CFrmCodeEdit::onStartEditLine(QLineEdit* pLine, QString sLabelText)
{
    if(!_pKeyboard) {
        _pKeyboard = new CDlgFullKeyboard();
        connect(_pKeyboard, SIGNAL(__TextEntered(CDlgFullKeyboard*,CCurrentEdit*)),
                this, SLOT(OnKeyboardTextEntered(CDlgFullKeyboard*, CCurrentEdit*)));
    }
    if(!_pcurrentLineEdit) {
        _pcurrentLineEdit = new CCurrentEdit();
    }
    if(_pKeyboard && _pcurrentLineEdit) {
        _pKeyboard->setCurrentEdit(_pcurrentLineEdit);
        _pcurrentLineEdit->setLineToBeEdited(pLine);
        _pcurrentLineEdit->setOriginalTextToEdit(pLine->text());
        _pcurrentLineEdit->setLabelText(sLabelText);

        _pKeyboard->setActive();
    }
}

void CFrmCodeEdit::OnKeyboardTextEntered(CDlgFullKeyboard *keyboard, CCurrentEdit *currEdit)
{
    currEdit->getLineBeingEdited()->setText(currEdit->getNewText());
    keyboard->hide();
}

void CFrmCodeEdit::on_buttonBox_clicked(QAbstractButton *button)
{
    //
}

void CFrmCodeEdit::on_chkAllAccess_clicked(bool checked)
{
    if(checked) {
        ui->dtStartAccess->setDateTime(QDateTime(QDate(1990,1,1), QTime(0,0,0)));
        ui->dtEndAccess->setDateTime(QDateTime(QDate(1990,1,1), QTime(0,0,0)));
        ui->dtStartAccess->hide();
        ui->dtEndAccess->hide();
        ui->strStartAccess->show();
        ui->strEndAccess->show();
    } else {
        ui->strStartAccess->hide();
        ui->strEndAccess->hide();
        ui->dtStartAccess->show();
        ui->dtEndAccess->show();
    }
}

void CFrmCodeEdit::on_chkFingerPrint1_clicked(bool checked)
{
    if( ui->chkFingerPrint2->isChecked() )
        ui->chkFingerPrint2->setChecked(false);

    ui->edtAccessCode->setDisabled(false);
    ui->edtSecondCode->setDisabled(false);
    if( ui->chkFingerPrint1->isChecked() )
    {
        if( ui->edtAccessCode->text() == QString("") )
        {
            int nRC_codeOne = QMessageBox::warning(this, tr("No Code #1 Entered"),
                                                   tr("You are enabling fingerprint authentication for this code,"
                                                      " but you must have already entered a Code #1."
                                                      " Enter a unique Code #1 and try again."),
                                                   QMessageBox::Ok);
            ui->chkFingerPrint1->setChecked(false);
            return;
        }

        //NEED TO CHECK HERE IF THERE'S ALREADY A FINGERPRINT FOR THIS CODE

        int nRC = QMessageBox::warning(this, tr("Warning! Adding Fingerprint Authentication"),
                                       tr("You are enabling fingerprint authentication for this code,"
                                          " Are you sure this is what you want to do?"),
                                       QMessageBox::Yes, QMessageBox::Cancel);

        if( nRC == QMessageBox::Yes )
        {
            ui->edtAccessCode->setDisabled(true);
            ui->chkFingerPrint1->setChecked(true);
        }
        else
        {
            ui->chkFingerPrint1->setChecked(false);
        }

    }
    else
    {
        QString printDirectory = "/home/pi/run/prints/" + ui->edtAccessCode->text();
        if( QDir(printDirectory).exists() )
        {
            int nRC = QMessageBox::warning(this, tr("Verify Fingerprint Scan Removal"),
                                           tr("Saved fingerprint scan exists for this code.\nDo you really want to remove it?"),
                                           QMessageBox::Yes, QMessageBox::Cancel);
            if(nRC == QMessageBox::Yes) {
                std::system( ("sudo rm -rf " + printDirectory.toStdString()).c_str());
            }
            else
            {
                ui->chkFingerPrint1->setChecked(true);
                return;
            }

        }

        ui->edtAccessCode->setDisabled(false);
    }
}

void CFrmCodeEdit::on_chkQuestions_clicked(bool checked)
{
    qDebug() << "CFrmCodeEdit::on_chkQuestions_clicked()";
    if(!_pDlgEditQuestions)
    {
        _pDlgEditQuestions = new CDlgEditQuestions(this);
        connect(_pDlgEditQuestions, SIGNAL(__OnQuestionEditSave()), this, SLOT(OnQuestionEditSave()));
        connect(_pDlgEditQuestions, SIGNAL(__OnQuestionEditClose()), this, SLOT(OnQuestionEditClose()));
        _pDlgEditQuestions->setValues(_question1, _question2, _question3);
        _pDlgEditQuestions->show();


    }
}

void CFrmCodeEdit::on_chkFingerPrint2_clicked(bool checked)
{
    if( ui->chkFingerPrint1->isChecked() )
        ui->chkFingerPrint1->setChecked(false);
    ui->edtAccessCode->setDisabled(false);

    if( ui->chkFingerPrint2->isChecked() )
    {
        // NEED CONFIRMATION IF ENROLLMENT EXISTS

        ui->edtSecondCode->setDisabled(true);

    }
    else
    {
        QString printDirectory = "/home/pi/run/prints/" + ui->edtSecondCode->text();
        if( QDir(printDirectory).exists() )
        {
            int nRC = QMessageBox::warning(this, tr("Verify Fingerprint Scan Removal"),
                                           tr(" Saved fingerprint scan exists for this code.\nDo you really want to remove it?"),
                                           QMessageBox::Yes, QMessageBox::Cancel);
            if(nRC == QMessageBox::Yes) {
                std::system(("sudo rm -rf " + printDirectory.toStdString()).c_str());
                ui->edtSecondCode->setDisabled(false);
            }
        }

        ui->edtSecondCode->setDisabled(false);
    }
}

void CFrmCodeEdit::OnAdminInfoCodes(QString code1, QString code2)
{
    emit __OnCodes(code1);
}

void CFrmCodeEdit::on_clrAccessCode_clicked()
{
    qDebug() << "on_clrAccessCode_clicked()";
    ui->edtAccessCode->setText("");
}

void CFrmCodeEdit::on_clrSecondCode_clicked()
{
    qDebug() << "on_clrSecondCode_clicked()";
    ui->edtSecondCode->setText("");
}

void CFrmCodeEdit::on_clrDescription_clicked()
{
    qDebug() << "on_clrDescription_clicked()";
    ui->edtDescription->setText("");
}

void CFrmCodeEdit::OnQuestionEditSave()
{
    qDebug() << "CFrmAdminInfo::OnQuestionEditSave()";
    if(_pDlgEditQuestions)
    {
        _pDlgEditQuestions->getValues(&_question1, &_question2, &_question3);
    }
}

void CFrmCodeEdit::OnQuestionEditClose()
{
    qDebug() << "CFrmAdminInfo::OnQuestionEditClose()";
    if(_pDlgEditQuestions)
    {
        _pDlgEditQuestions->close();
        delete _pDlgEditQuestions;
        _pDlgEditQuestions = 0;
    }
}
