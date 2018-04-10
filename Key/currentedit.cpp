#include "currentedit.h"

CCurrentEdit::CCurrentEdit()
{
    pregExpNums = new QRegExpValidator(QRegExp("[0-9]*$"));

    QRegExp mailREX(QRegExp("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b"));
    mailREX.setCaseSensitivity(Qt::CaseInsensitive);
    mailREX.setPatternSyntax(QRegExp::RegExp);
    pregExpEmail = new QRegExpValidator(mailREX);

    _sInputMask = "";
}

bool CCurrentEdit::isNumbersOnly()
{
    return _bNumbersOnly;
}

bool CCurrentEdit::setNumbersOnly(bool bNumsOnly)
{
    return _bNumbersOnly == bNumsOnly;
}

void CCurrentEdit::setKeyboardEditLine(QLineEdit *pEd)
{
    pEditLine = pEd;
    if( _bNumbersOnly )
    {
        pEditLine->setValidator(pregExpNums);
    } else
    {
        pEditLine->setValidator(NULL);
    }
}

void CCurrentEdit::setKeyboardLabel(QLabel *pLabel)
{
    pLabelTitle = pLabel;
}

void CCurrentEdit::setLabelToBeEdited(QLabel *pLabel)
{
    _pLineCurrentlyBeingEdited = NULL;      // Clear the label object
    pLabelCurrentlyBeingEdited = pLabel;
}


QLabel *CCurrentEdit::getLabelBeingEdited()
{
    return pLabelCurrentlyBeingEdited;
}

void CCurrentEdit::setLabelText(QString newValue)
{
    labelText = newValue;
    pLabelTitle->setText(labelText);
}

void CCurrentEdit::setLineToBeEdited(QLineEdit *pLine)
{
    pLabelCurrentlyBeingEdited = NULL;      // Clear the line edit object
    _pLineCurrentlyBeingEdited = pLine;
}

QLineEdit* CCurrentEdit::getLineBeingEdited()
{
    return _pLineCurrentlyBeingEdited;
}

QString CCurrentEdit::getLabelText()
{
    return labelText;
}

void CCurrentEdit::setOriginalTextToEdit( QString text )
{
    originalText = text;
    pEditLine->setText(originalText);
}

void CCurrentEdit::setNewText(QString text)
{
    newText = text;
}

QString CCurrentEdit::getNewText()
{
    return newText;
}

QString CCurrentEdit::retrieveEditedText()
{
    newText = pEditLine->text();
    return newText;
}

void CCurrentEdit::clearInputMasks()
{
    _sInputMask = "";
    _bNumbersOnly = false;
    emit __onNumbersOnly(false);
}

void CCurrentEdit::setNumbersOnly()
{
}

void CCurrentEdit::setEmailFilter()
{
}

void CCurrentEdit::setMaxLength(int nLen)
{
    pEditLine->setMaxLength(nLen);
}

void CCurrentEdit::stopEdit()
{
}

void CCurrentEdit::OnStringEntry(QString newValue)
{
    setOriginalTextToEdit(newValue);
}
