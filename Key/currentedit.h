#ifndef CCURRENTEDIT_H
#define CCURRENTEDIT_H

#include <QObject>
#include <QRegExp>
#include <QRegExpValidator>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>

class CCurrentEdit : public QObject
{
    Q_OBJECT
private:
    bool _bNumbersOnly = false;

public:
    explicit CCurrentEdit()
    {
        pregExpNums = new QRegExpValidator(QRegExp("[0-9]*$"));

        QRegExp mailREX(QRegExp("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b"));
        mailREX.setCaseSensitivity(Qt::CaseInsensitive);
        mailREX.setPatternSyntax(QRegExp::RegExp);
        pregExpEmail = new QRegExpValidator(mailREX);

        _sInputMask = "";
    }

    QObject             *parent;
    QString             labelText;
    QString             originalText;
    QString             newText;

    QRegExpValidator    *pregExpNums;
    QRegExpValidator    *pregExpEmail;

    QLabel              *pLabelTitle;

    QLineEdit           *pEditLine;

    QString             _sInputMask;
    QLabel              *pLabelCurrentlyBeingEdited;
    QLineEdit           *_pLineCurrentlyBeingEdited;

    bool isNumbersOnly() { return _bNumbersOnly; }
    bool setNumbersOnly(bool bNumsOnly) { return _bNumbersOnly == bNumsOnly; }

    void setKeyboardEditLine(QLineEdit *pEd)
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

    void setKeyboardLabel(QLabel *pLabel)
    {
        pLabelTitle = pLabel;
    }

    void setLabelToBeEdited(QLabel *pLabel)
    {
        _pLineCurrentlyBeingEdited = NULL;      // Clear the label object
        pLabelCurrentlyBeingEdited = pLabel;
    }

    QLabel *getLabelBeingEdited()
    {
        return pLabelCurrentlyBeingEdited;
    }

    void setLabelText(QString newValue)
    {
        labelText = newValue;
        pLabelTitle->setText(labelText);
    }

    void setLineToBeEdited(QLineEdit *pLine)
    {
        pLabelCurrentlyBeingEdited = NULL;      // Clear the line edit object
        _pLineCurrentlyBeingEdited = pLine;
    }

    QLineEdit* getLineBeingEdited()
    {
        return _pLineCurrentlyBeingEdited;
    }

    QString getLabelText() { return labelText; }

    void setOriginalTextToEdit( QString text )
    {
        originalText = text;
        pEditLine->setText(originalText);
    }

    void setNewText(QString text) { newText = text; }

    QString getNewText() { return newText; }

    QString retrieveEditedText() { newText = pEditLine->text(); return newText; }

    void clearInputMasks()
    {
        _sInputMask = "";
        _bNumbersOnly = false;
        emit __onNumbersOnly(false);
    }

    void setNumbersOnly()
    {
    }

    void setEmailFilter()
    {
    }

    void setMaxLength(int nLen)
    {
        pEditLine->setMaxLength(nLen);
    }

    void stopEdit()
    {
    }

signals:
    void __onNumbersOnly(bool bSet);

public slots:
    void OnStringEntry(QString newValue);
};

#endif // CCURRENTEDIT_H
