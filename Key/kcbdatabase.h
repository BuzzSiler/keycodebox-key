#ifndef DATABASE_H
#define DATABASE_H

#include <QScopedPointer>
#include <QStringList>
#include <QString>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QMap>
#include <QList>


namespace kcb
{
    class Database
    {
        public:
            #define TABLE_DICT_TYPE QMap<QString, QString>
            #define TABLE_FIELDS_TYPE QList<QString>
            #define ROW_DICT_TYPE QMap<QString, QVariant>

            explicit Database(QSqlDatabase& database, QString tablename, TABLE_DICT_TYPE fields);
            ~Database();

            QSqlQuery createQuery(QStringList column_list, QString condition);
            QStringList getFieldNames();
            bool tableExists();
            void createTable(TABLE_DICT_TYPE columns);
            bool updateRequired();
            bool columnExists(QString column);
            void createColumn(QString column, QString fieldType);
            bool insertInto(QStringList colmuns, QList<QVariant> values);
            bool insertIntoRow(ROW_DICT_TYPE row);
            bool update(QStringList columns, QList<QVariant> values, QVariant ids, QString condition = "");
            bool update(ROW_DICT_TYPE row, QVariant ids, QString condition = "");
            bool transaction();
            bool rollback();
            bool commit();
            bool incrementField(QString column, QVariant ids);
            bool deleteRow(QVariant ids);
            bool hasVersion();
            QStringList getFieldTypes();
            int getLastInsertId();

        protected:
            QSqlDatabase& m_database;
            QSqlQueryModel& m_readonly_model;
            QSqlTableModel& m_model;

            QString m_tablename;
            TABLE_DICT_TYPE m_table;
            int m_last_inserted_id;

            void setLastInsertId(QSqlQuery query);

    };
}
#endif // DATABASE_H
