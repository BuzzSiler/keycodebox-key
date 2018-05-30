#include "checkablestringlistmodel.h"

#include <QColor>

CheckableStringListModel::CheckableStringListModel(QObject* parent)
{

}

CheckableStringListModel::CheckableStringListModel(const QStringList & strings, QObject* parent)
{

}

Qt::ItemFlags CheckableStringListModel::flags (const QModelIndex & index) const
{
    Qt::ItemFlags defaultFlags = QStringListModel::flags(index);
    if (index.isValid())
    {
        return defaultFlags | Qt::ItemIsUserCheckable;
    }
    return defaultFlags;
}

bool CheckableStringListModel::setData(const QModelIndex &index,
                                const QVariant &value, int role)
{

    if(!index.isValid() || role != Qt::CheckStateRole)
    {
        return false;
    }

    if(value == Qt::Checked)
    {
        checkedItems.insert(index);
    }
    else
    {
        checkedItems.remove(index);
    }

    emit dataChanged(index, index);
    return true;
}

QVariant CheckableStringListModel::data(const QModelIndex &index,
                                 int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if(role == Qt::CheckStateRole)
    {
        return checkedItems.contains(index) ?
                    Qt::Checked : Qt::Unchecked;
    }
    else if(role == Qt::BackgroundColorRole)
    {
        return checkedItems.contains(index) ?
                    QColor("#ffffb2") : QColor("#ffffff");
    }

    return QStringListModel::data(index, role);
}
