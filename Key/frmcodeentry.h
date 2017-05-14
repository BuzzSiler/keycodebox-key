#ifndef FRMCODEENTRY_H
#define FRMCODEENTRY_H

#include <QWidget>

namespace Ui {
class CFrmCodeEntry;
}

class CFrmCodeEntry : public QWidget
{
    Q_OBJECT

public:
    explicit CFrmCodeEntry(QWidget *parent = 0);
    ~CFrmCodeEntry();

private:
    Ui::CFrmCodeEntry *ui;
};

#endif // FRMCODEENTRY_H
