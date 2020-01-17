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

    namespace Utils
    {
        unsigned int countSetBits(int n);
        QPixmap CreateQrCode(const QByteArray& data);
        QChar GetRandomChar();
        QStringList CreateRandomValues(int num_values, int code_length);
        QString JsonToString(const QJsonObject& json);
        QJsonObject StringToJson(const QString& str);
        QDateTime StringToDateTime(const QString& datetime);
        QJsonObject JsonFromFile(QString const &filename);
        void JsonToFile(QString const &filename, QJsonObject const &json);

        void setIntValue(QJsonObject& json, QString const & key, int value);
        int getIntValue(QJsonObject& json, QString const & key, int const & default_value = 0);
        void setBoolValue(QJsonObject& json, QString const & key, bool value);
        bool getBoolValue(QJsonObject& json, QString const & key, bool const & default_value = false);
        void setStringValue(QJsonObject& json, QString const & key, QString value);
        QString getStringValue(QJsonObject& json, QString const & key, QString const & default_value = QString(""));
        bool fileExists(QString const & filename);


    }
}
#endif // KCBUTILS_H
