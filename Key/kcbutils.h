#ifndef KCBUTILS_H
#define KCBUTILS_H

#include <QObject>
#include <QString>
#include <memory>
#include <QPixmap>
#include <QByteArray>

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
    QPixmap CreateQrCode(const QByteArray& data);
    QChar GetRandomChar();
    QStringList CreateRandomValues(int num_values, int code_length);
    QString JsonToString(const QJsonObject& json);
    QJsonObject StringToJson(const QString& str);
    QDateTime StringToDateTime(const QString& datetime);

}
#endif // KCBUTILS_H
