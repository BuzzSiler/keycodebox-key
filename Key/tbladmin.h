#ifndef CTBLADMIN_H
#define CTBLADMIN_H

#include <ctime>
#include <string>
#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QSqlQuery>
#include "adminrec.h"

class QSqlDatabase;
class QJsonObject;
class QString;
class QStringList;



class CTblAdmin
{
    const QString TABLENAME = "admin";

    private:
        QSqlDatabase *_pDB;

        bool createTable();
        bool tableExists();
        void createColumn(QString column, QString fieldType, QString value);
        bool columnExists(QString column);

        CAdminRec   _currentAdmin;
        QString _version;

        void currentTimeFull(QString strBuffer);
        void currentTimeFormat(QString format, QString strBuffer, int nExpectedLength);
        bool createAdminDefault();
        void initialize();
        bool readAdmin();
        int queryCommonFields(const QStringList& common_fields, QMap<QString, QString>& dict);
        QSqlQuery createQuery(const QStringList& column_list,
                              const QString& table);
        int dropTable();
        bool getVersion();
        QStringList getCommonFields();
        void populateAdminWithDefaults();
        void updateAdminFromCommonFields(const QMap<QString, QString>& fieldValues);
        QMap<QString, QString> mergeCommonFields(const QStringList& fields);


    public:
        CTblAdmin(QSqlDatabase *db);

        void addAdmin();
        bool updateAdmin(QString name, QString email, QString phone,
                        QDateTime repFreq, QDateTime startReport,
                        QString passwordEnc,
                        QString assistPasswordEnc,
                        bool showFingerprint, bool showPassword,
                        uint32_t nMaxLocks,
                        QString smtpserver, int smptport, int smtptype,
                        QString smtpusername, QString smtppassword,
                        int vncport, QString vncpasword,
                        bool bEmailReport, bool bSaveReport, QString reportDirectory,
                        int displayPowerDownTimeout,
                        QDateTime reportDeletion, bool showTakeReturn);

        bool updateAdminClear(QString name, QString email, QString phone,
                        QDateTime repFreq, QDateTime startReport,
                        QString passwordClear,
                        QString assistPasswordClear,
                        bool showFingerprint, bool showPassword,
                        uint32_t nMaxLocks,
                        QString smtpserver, int smptport, int smtptype,
                        QString smtpusername, QString smtppassword,
                        int vncport, QString vncpassword,
                        bool bEmailReport, bool bSaveReport, QString reportDirectory,
                        int displayPowerDownTimeout,
                        QDateTime reportDeletion, bool showTakeReturn);
        bool updateAdmin(CAdminRec &rec);
        bool updateAdmin(QJsonObject adminObj);

        time_t  getReportStartTime();
        time_t  getReportFrequency();

        QString isAccessCode(QString code);     // Numeric
        bool isPassword(QString password, QString type);

        CAdminRec &getCurrentAdmin() { return _currentAdmin; }
};

#endif // CTBLADMIN_H
