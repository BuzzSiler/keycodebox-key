#include "cabinetrowdelegate.h"


#include <QSpinBox>
#include <QComboBox>
#include <QStringList>

#include "logger.h"

static int const TOTAL_LOCKS_COL = 3;

CabinetRowDelegate::CabinetRowDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *CabinetRowDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    Q_UNUSED(option);

    if (index.column() == TOTAL_LOCKS_COL)
    {
        QComboBox *editor = new QComboBox(parent);
        connect(editor, SIGNAL(currentIndexChanged(int)), this, SLOT(commitAndCloseEditor(int)));
        editor->addItems(QStringList() << "8" << "16" << "32");
        return editor;
    }

    return nullptr;
}

void CabinetRowDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    if (index.column() == TOTAL_LOCKS_COL)
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentText(QString::number(value));
    }
}

void CabinetRowDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    if (index.column() == TOTAL_LOCKS_COL)
    {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int value = comboBox->currentText().toInt();
        model->setData(index, value, Qt::EditRole);
    }
}

void CabinetRowDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}

void CabinetRowDelegate::commitAndCloseEditor(int index)
{
    Q_UNUSED(index);
     QComboBox *editor = qobject_cast<QComboBox *>(sender());
     emit commitData(editor);
     emit closeEditor(editor);
}