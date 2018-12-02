#include "kcbdatabase.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlRecord>
#include "kcbcommon.h"

static const QString CREATE_TABLE_CMD = QString("CREATE TABLE IF NOT EXISTS");

namespace kcb
{
    Database::Database(QSqlDatabase& database, QString tablename, TABLE_DICT_TYPE table)
        : m_database(database),
          m_readonly_model(*new QSqlQueryModel()),
          m_model(*new QSqlTableModel(Q_NULLPTR, database)),
          m_tablename(tablename),
          m_table(table),
          m_last_inserted_id(-1)
    {
        KCB_DEBUG_TRACE("Instantiating: " << m_tablename);
    }

    Database::~Database()
    {
    }

    //---------------------------------------------------------------------------------------------
    // Decsription:
    // Creates an SQL query using the specified column_list and condition.  If condition is empty
    // no where clause is added.
    //---------------------------------------------------------------------------------------------
    QSqlQuery Database::createQuery(QStringList column_list, QString condition)
    {
        KCB_DEBUG_ENTRY;

        bool is_open = m_database.isOpen();

        KCB_DEBUG_TRACE("database is open" << is_open);
        Q_ASSERT_X(is_open, Q_FUNC_INFO, "database is not open");

        if (!is_open)
        {
            return QSqlQuery();
        }

        QSqlQuery query(m_database);
        QString sql;
        
        query.setForwardOnly(true);

        KCB_DEBUG_TRACE("column list count" << column_list.count());
        KCB_DEBUG_TRACE("condition is empty" << condition.isEmpty());

        auto select = QString("SELECT *");
        if (column_list.count() > 0)
        {
            select = QString("SELECT %1").arg(column_list.join(","));
        }
        sql += QString("%1").arg(select);
        auto from = QString("FROM %1").arg(m_tablename);
        sql += QString(" %1").arg(from);
        if (!condition.isEmpty())
        {
            auto where = QString("WHERE %1").arg(condition);
            sql += QString(" %1").arg(where);
        }

        KCB_DEBUG_TRACE("SQL:" << sql);

        if( !query.prepare(sql) )
        {
            KCB_WARNING_TRACE("prepare failed" << query.lastError());
        }

        KCB_DEBUG_EXIT;

        return query;
    }

    QStringList Database::getFieldNames()
    {
        QStringList fields;
        auto qry = createQuery(QStringList(), QString());
        if (qry.exec())
        {
            auto rec = qry.record();
            KCB_DEBUG_TRACE("Number of columns: " << rec.count());
            for (int ii = 0; ii < rec.count(); ++ii)
            {
                fields.append(rec.fieldName(ii));
            }

            KCB_DEBUG_TRACE("Field Names: " << fields);
        }

        return fields;
    }

    bool Database::tableExists()
    {
        KCB_DEBUG_ENTRY;

        bool is_open = m_database.isOpen();

        Q_ASSERT_X(is_open, Q_FUNC_INFO, "database pointer is null");
        
        if (!is_open)
        {
            return false;
        }

        KCB_DEBUG_TRACE(m_tablename);

        QStringList tables = m_database.tables();

        foreach (auto table, tables)
        {
            if (table == m_tablename)
            {
                KCB_DEBUG_EXIT;
                return true;
            }
        }

        KCB_DEBUG_EXIT;
        return false;
    }

    void Database::createTable(TABLE_DICT_TYPE table)
    {
        KCB_DEBUG_ENTRY;

        bool is_open = m_database.isOpen();

        Q_ASSERT_X(is_open, Q_FUNC_INFO, "database is not open");
        
        if( !is_open ) 
        {
            KCB_DEBUG_TRACE("database is not open");
            return;
        }

        KCB_DEBUG_TRACE("Creating table" << m_tablename);

        QSqlQuery query(m_database);

        // Create columns with types
        QStringList columns;
        foreach (auto key, table.keys())
        {
            columns << QString("%1 %2").arg(key).arg(table[key]);
        }

        QString sql;
        sql += QString("%1 %2 (%3)").arg(CREATE_TABLE_CMD).arg(m_tablename).arg(columns.join(","));

        KCB_DEBUG_TRACE("SQL:" << sql);

        if( !query.prepare(sql) )
        {
            KCB_WARNING_TRACE("prepare failed" << query.lastError());
        }

        if( !query.exec() )
        {
            KCB_WARNING_TRACE("exec failed" << query.lastError());
        }

        KCB_DEBUG_TRACE("Table successfully created!");
    }

    bool Database::updateRequired()
    {
        QStringList fields = getFieldNames();
        QStringList types = getFieldTypes();

        bool same_num_fields = fields.count() == m_table.keys().count();
        bool diff_fields = false;
        bool diff_types = false;

        if (same_num_fields)
        {
            QSet<QString> fields_set = m_table.keys().toSet().subtract(fields.toSet());

            diff_fields = fields_set.count() > 0;

            KCB_DEBUG_TRACE(fields_set);

            QSet<QString> types_set = m_table.values().toSet().subtract(types.toSet());

            // Note: ignored the type of the ids field.  It goes in as "integer primary key unique" but
            // comes back in a different format.
            if (types_set.count() > 0 && !types_set.contains("integer primary key unique"))
            {
                diff_types = true;
                KCB_DEBUG_TRACE(types_set);
            }
        }

        KCB_DEBUG_TRACE("Diff Counts" << !same_num_fields);
        KCB_DEBUG_TRACE("Diff Fields" << diff_fields);
        KCB_DEBUG_TRACE("Diff Types" << diff_types);

        return !same_num_fields || diff_fields || diff_types;
    }

    bool Database::columnExists(QString column)
    {
        auto fields = getFieldNames();

        foreach (auto field, fields)
        {
            if (field == column)
            {
                KCB_DEBUG_TRACE(column << "exists");
                return true;
            }
        }

        KCB_DEBUG_TRACE(column << "does not exist");
        return false;
    }

    QStringList Database::getFieldTypes()
    {
        QString sql(QString("PRAGMA TABLE_INFO(%1);").arg(m_tablename));

        QSqlQuery query(m_database);

        if (!query.prepare( sql ))
        {
            return QStringList();
        }

        if (!query.exec())
        {
            return QStringList();
        }

        QStringList types;
        while (query.next())
        {
            // Returns: column id, column name, data type, whether or not the column can be NULL, default value for the column, is primary key
            KCB_DEBUG_TRACE(query.value(2).toString());
            types << query.value(2).toString();
        }

        return types;
    }

    void Database::createColumn(QString column, QString fieldType)
    {
        QSqlQuery query(m_database);

        QString sql(QString("ALTER TABLE %1 ADD %2 %3").arg(m_tablename, column, fieldType));

        KCB_DEBUG_TRACE("SQL:" << sql);

        if (!query.prepare( sql ))
        {
            KCB_DEBUG_TRACE("prepare failed" << query.lastError());
        }

        if( !query.exec() )
        {
            KCB_DEBUG_TRACE("exec failed" << query.lastError());
        }

        KCB_DEBUG_TRACE("Table successfully altered!");
    }

    bool Database::insertInto(QStringList columns, QList<QVariant> values)
    {
        QVector<QString> placeholders(columns.count());
        placeholders.fill("?");
        
        QString sql(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(m_tablename).arg(columns.join(',')).arg(QStringList(placeholders.toList()).join(',')));

        KCB_DEBUG_TRACE("SQL:" << sql);

        QSqlQuery query(m_database);
        query.prepare(sql);
        
        foreach (auto entry, values)
        {
            query.addBindValue(entry);
        }
        
        bool result = query.exec();
        setLastInsertId(query);

        if (result)
        {
            KCB_DEBUG_TRACE("data inserted");
            return true;
        }

        KCB_DEBUG_TRACE("exec failed" << query.lastError());
        KCB_DEBUG_TRACE("last query" << query.lastQuery());
        return false;
    }

    bool Database::insertIntoRow(ROW_DICT_TYPE row)
    {
        QStringList values;
        foreach (auto key, row.keys())
        {
            values.append(QString(":%1").arg(key));
        }

        QString sql(QString("INSERT INTO %1 (%2) VALUES (%3)").arg(m_tablename).arg(QStringList(row.keys()).join(',')).arg(values.join(',')));
        KCB_DEBUG_TRACE("SQL:" << sql);

        QSqlQuery query(m_database);

        query.prepare(sql);

        foreach (auto key, row.keys())
        {
            query.bindValue(QString(":%1").arg(key), row[key]);
        }

        bool result = query.exec();        
        setLastInsertId(query);

        if (result)
        {
            KCB_DEBUG_TRACE("data inserted");
            return true;
        }

        KCB_DEBUG_TRACE("exec failed" << query.lastError());
        KCB_DEBUG_TRACE("last query" << query.lastQuery());
        return false;

    }

    bool Database::update(QStringList columns, QList<QVariant> values, QVariant ids, QString condition)
    {
        QStringList placeholders;
        QSqlQuery query(m_database);

        Q_ASSERT_X(columns.count() == values.count(), Q_FUNC_INFO, "column/value mismatch");
        if (columns.count() != values.count())
        {
            KCB_DEBUG_TRACE("Col: " << columns.count() << "Val: " << values.count());
            return false;
        }

        //-----------------------------------------------------------------------------------------
        // Create SQL
        //    UPDATE <tablename> SET <placeholders> <where>
        //-----------------------------------------------------------------------------------------

        foreach (auto entry, columns)
        {
            placeholders.append(QString("%1=?").arg(entry));
        }

        // Default 'where' to be empty
        QString where("");

        // If ids are specified then create placeholder
        if (!ids.isNull())
        {
            where = QString("WHERE ?");
        }
        
        // If condition is specified then add it
        if (!condition.isEmpty())
        {
            if (where.isEmpty())
            {
                where += QString("%1").arg(condition);
            }
            else
            {            
                where += QString("AND %1").arg(condition);
            }
        }

        // Fill in the SQL text
        QString sql(QString("UPDATE %1 SET %2 %3").arg(m_tablename).arg(placeholders.join(',')).arg(where));

        KCB_DEBUG_TRACE("SQL:" << sql);

        if (!query.prepare(sql))
        {
            KCB_DEBUG_TRACE("prepare failed" << query.lastError());
            KCB_DEBUG_TRACE("last query" << query.lastQuery());
            return false;
        }

        foreach (auto entry, values)
        {
            query.addBindValue(entry);
        }

        if (!ids.isNull())
        {
            query.addBindValue(ids);
        }

        if ( !query.exec() ) 
        {
            KCB_DEBUG_TRACE("exec failed" << query.lastError());
            KCB_DEBUG_TRACE("last query" << query.lastQuery());
            return false;
        }

        KCB_DEBUG_TRACE("exec succeeded (active:" << query.isActive() << ")");
        return true;
    }

    bool Database::update(ROW_DICT_TYPE row, QVariant ids, QString condition)
    {
        QStringList colname_list;
        foreach (auto key, row.keys())
        {
            colname_list.append(QString("%1=:%1").arg(key));
        }

        // Default 'where' to be empty
        QString where("");

        // If ids are specified then create placeholder
        if (!ids.isNull())
        {
            where = QString("WHERE ids=:ids");
        }
        
        // If condition is specified then add it
        if (!condition.isEmpty())
        {
            if (where.isEmpty())
            {
                where += QString("%1").arg(condition);
            }
            else
            {            
                where += QString("AND %1").arg(condition);
            }
        }

        // Create the SQL string
        QString sql(QString("UPDATE %1 SET %2 %3").arg(m_tablename).arg(colname_list.join(',')).arg(where));
        KCB_DEBUG_TRACE("SQL:" << sql);

        // Create the query
        QSqlQuery query(m_database);

        if (!query.prepare(sql))
        {
            KCB_DEBUG_TRACE("prepare failed");
            return false;
        }

        // Bind variables

        // Bind column values        
        foreach (auto key, row.keys())
        {
            query.bindValue(QString(":%1").arg(key), row[key]);
        }

        // Bind ids (if present)
        if (!ids.isNull())
        {
            query.bindValue(QString(":ids"), ids);
        }

        // Execute the query and store the insert id
        bool result = query.exec();        
        setLastInsertId(query);

        if (result)
        {
            KCB_DEBUG_TRACE("data inserted");
            return true;
        }

        KCB_DEBUG_TRACE("exec failed" << query.lastError());
        KCB_DEBUG_TRACE("last query" << query.lastQuery());
        return false;
    }


    bool Database::transaction()
    {
        return m_database.transaction();
    }

    bool Database::rollback()
    {
        return m_database.rollback();
    }

    bool Database::commit()
    {
        return m_database.commit();
    }

    bool Database::incrementField(QString column, QVariant ids)
    {
        QString field_increment= QString("%1 = %1 + 1").arg(column);
        QString sql = QString("UPDATE %1 SET %2 WHERE ids=?").arg(m_tablename).arg(field_increment);

        QSqlQuery query(m_database);

        query.prepare(sql);

        query.addBindValue(ids);

        if ( query.exec() ) 
        {
            KCB_DEBUG_TRACE("field incremented");
            return true;
        }

        KCB_DEBUG_TRACE("exec failed" << query.lastError());
        KCB_DEBUG_TRACE("last query" << query.lastQuery());
        return false;
    }

    bool Database::deleteRow(QVariant ids)
    {
        QSqlQuery query(m_database);
        QString sql = QString("DELETE FROM %1 WHERE ids=?").arg(m_tablename);

        query.prepare(sql);
        query.addBindValue(ids);
        if( query.exec()) 
        {
            KCB_DEBUG_TRACE("delete succeeded");
            return true;
        } 
        else 
        {
            KCB_DEBUG_TRACE("delete failed");
            return false;
        }
    }

    bool Database::hasVersion()
    {
        foreach (auto field, getFieldNames())
        {
            if (field == "version")
            {
                return true;
            }
        }

        return false;
    }

    void Database::setLastInsertId(QSqlQuery query)
    {
        QVariant var = query.lastInsertId();
        if (var.isValid())
        {
            m_last_inserted_id = var.toInt();
        }
    }

    int Database::getLastInsertId()
    {
        return m_last_inserted_id;
    }

}
