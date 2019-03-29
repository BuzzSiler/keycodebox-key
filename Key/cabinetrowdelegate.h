#ifndef CABINETROWDELEGATE_H
#define CABINETROWDELEGATE_H

#include <QStyledItemDelegate>


class CabinetRowDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        CabinetRowDelegate(QObject *parent = 0);

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const override;

        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model,
                        const QModelIndex &index) const override;

        void updateEditorGeometry(QWidget *editor,
            const QStyleOptionViewItem &option, const QModelIndex &index) const override;

        void SetRange(int lower_bound, int upper_bound);
};

#endif //CABINETROWDELEGATE_H