#ifndef IMAGEDELEGATE_H
#define IMAGEDELEGATE_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

class ImageDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ImageDelegate(QObject *parent = 0);
    
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

};

#endif
