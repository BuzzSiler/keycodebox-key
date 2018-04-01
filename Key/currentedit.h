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
    explicit CCurrentEdit();

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

    bool isNumbersOnly();
    bool setNumbersOnly(bool bNumsOnly);

    void setKeyboardEditLine(QLineEdit *pEd);

    void setKeyboardLabel(QLabel *pLabel);

    void setLabelToBeEdited(QLabel *pLabel);

    QLabel *getLabelBeingEdited();

    void setLabelText(QString newValue);

    void setLineToBeEdited(QLineEdit *pLine);

    QLineEdit* getLineBeingEdited();

    QString getLabelText();

    void setOriginalTextToEdit( QString text );

    void setNewText(QString text);

    QString getNewText();

    QString retrieveEditedText();

    void clearInputMasks();

    void setNumbersOnly();

    void setEmailFilter();

    void setMaxLength(int nLen);

    void stopEdit();

signals:
    void __onNumbersOnly(bool bSet);

public slots:
    void OnStringEntry(QString newValue);
};

#endif // CCURRENTEDIT_H
