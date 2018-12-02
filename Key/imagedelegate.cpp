#include "imagedelegate.h"

#include <QTimeEdit>
#include <QString>
#include <QTime>
#include <QStaticText>
#include "kcbcommon.h"

const int IMAGE_COLUMN = 2;


ImageDelegate::ImageDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ImageDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    //KCB_DEBUG_ENTRY;
    //KCB_DEBUG_TRACE("row" << index.row() << "col" << index.column());
    if (index.column() == IMAGE_COLUMN)
    {
        QByteArray ba = index.data().toByteArray();
        //KCB_DEBUG_TRACE("ba size" << ba.size());
        if (ba.size() > 0)
        {
            QPixmap pm;
            pm.loadFromData(ba);
            pm = pm.scaled(option.rect.width(), option.rect.height(), Qt::KeepAspectRatio);

            const int x = option.rect.center().x() - pm.rect().width() / 2;
            const int y = option.rect.center().y() - pm.rect().height() / 2;
            //KCB_DEBUG_TRACE("x" << x << "y" << y);

            QRect position = QRect(x, y, pm.rect().width(), pm.rect().height());

            painter->drawPixmap(position, pm);//.scaled(25, 25, Qt::KeepAspectRatio));
        }
        else
        {
            painter->drawText(option.rect, Qt::AlignCenter, QString("No Image")); 
        }
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
    //KCB_DEBUG_EXIT;
}
