#ifndef KCBUTILS_H
#define KCBUTILS_H

#include <QObject>
#include <QString>
#include <memory>

namespace kcb
{
    class ClassAllocation
    {
        public:
            ClassAllocation();
            static void DestructorMsg(const QString& value);
            static void DestructorMsg(const QObject * const object);

            template<typename T, typename... Args>
            static std::unique_ptr<T> make_unique(Args&&... args);

        private:
            explicit ClassAllocation(const ClassAllocation& rhs) = delete;
            ClassAllocation& operator= (const ClassAllocation& rhs) = delete;
    };

    unsigned int countSetBits(int n);

}
#endif // KCBUTILS_H
