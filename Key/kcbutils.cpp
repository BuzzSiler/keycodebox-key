#include "kcbutils.h"
#include <QString>
#include <QDebug>

namespace kcb
{

    static QString DESTRUCTOR_MSG = QStringLiteral("Running the %1 destructor.");


    void ClassAllocation::DestructorMsg(const QString& value)
    {
        qDebug() << DESTRUCTOR_MSG.arg(value);
    }

    void ClassAllocation::DestructorMsg(const QObject * const object)
    {
        DestructorMsg(object->metaObject()->className());
    }

    template<typename T, typename... Args>
    static std::unique_ptr<T> make_unique(Args&&... args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    unsigned int countSetBits(int n) 
    { 
        unsigned int count = 0; 
        while (n) 
        { 
            n &= (n-1) ; 
            count++; 
        } 
        return count; 
    }

}
