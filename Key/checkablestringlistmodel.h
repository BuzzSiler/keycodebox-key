#ifndef CHECKABLESTRINGLISTMODEL_H
#define CHECKABLESTRINGLISTMODEL_H

#include <QObject>
#include <QStringListModel>
#include <QVariant>
#include <QModelIndex>
#include <QStringList>
#include <QSet>
#include <QPersistentModelIndex>

class CheckableStringListModel : public QStringListModel
{
    public:
        CheckableStringListModel(QObject* parent = 0);
        CheckableStringListModel(const QStringList & strings, QObject* parent = 0);
        Qt::ItemFlags flags (const QModelIndex& index) const;
        QVariant data(const QModelIndex &index, int role) const;
        bool setData(const QModelIndex &index, const QVariant &value,
                     int role);
        void save();
    private:
        QSet<QPersistentModelIndex> checkedItems;
};

#endif // CHECKABLESTRINGLISTMODEL_H
