#ifndef CABINET_TYPE_H
#define CABINET_TYPE_H

#include <QVector>
#include <QString>

namespace kcb
{
    struct CABINET_INFO
    {
        QString  model;
        uint16_t num_locks;
        uint16_t start;
        uint16_t stop;
        QString sw_version;
        QString addr;
        uint16_t max_locks;
        bool operator<(CABINET_INFO const & a) const
        {
            return start < a.start;
        }
    };

    bool operator==(CABINET_INFO const & lhs, CABINET_INFO const & rhs);

    typedef QVector<CABINET_INFO> CABINET_COLLECTION;

}

#endif // CABINET_TYPE_H