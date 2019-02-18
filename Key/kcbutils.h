#ifndef KCBUTILS_H
#define KCBUTILS_H

#include <QObject>
#include <memory>

class QString;
class QJsonObject;

namespace kcb
{
    class Utils
    {
        public:
            Utils();
            static void ConstructorMsg(const QString& value);
            static void ConstructorMsg(const QObject * const object);
            static void DestructorMsg(const QString& value);
            static void DestructorMsg(const QObject * const object);

            template<typename T, typename... Args>
            static std::unique_ptr<T> make_unique(Args&&... args)
            {
                return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
            }

            static void JsonFromFile(QString const& path, QJsonObject& json_obj);
            static void JsonToFile(QString const& path, QJsonObject& json_obj);

        private:
            explicit Utils(const Utils& rhs) = delete;
            Utils& operator= (const Utils& rhs) = delete;
    };
}
#endif // KCBUTILS_H
