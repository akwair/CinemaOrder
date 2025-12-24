#include "database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

Database::Database(const QString &path)
    : m_path(path)
{
}

Database::~Database()
{
    if (m_db.isOpen()) m_db.close();
}

bool Database::open()
{
    if (QSqlDatabase::contains("cinema_connection"))
        m_db = QSqlDatabase::database("cinema_connection");
    else
        m_db = QSqlDatabase::addDatabase("QSQLITE", "cinema_connection");

    m_db.setDatabaseName(m_path);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    return ensureTables();
}

QSqlDatabase Database::db() const
{
    return m_db;
}

bool Database::ensureTables()
{
    QSqlQuery q(m_db);
    const QString sql = R"(
    CREATE TABLE IF NOT EXISTS tickets (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        movieName TEXT,
        cinemaName TEXT,
        showDate TEXT,
        showTime TEXT,
        duration INTEGER,
        price REAL,
        hall TEXT,
        capacity INTEGER,
        remain INTEGER,
        sold INTEGER,
        movie_details TEXT
    )
)";
    if (!q.exec(sql)) {
        qWarning() << "Failed to create table:" << q.lastError().text();
        return false;
    }
    // users table for login/register
    const QString usersSql = R"(
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE,
        password_hash TEXT,
        role INTEGER DEFAULT 0
    )
)";
    if (!q.exec(usersSql)) {
        qWarning() << "Failed to create users table:" << q.lastError().text();
        return false;
    }
    
    // Check if role column exists, if not add it
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("PRAGMA table_info(users)");
    if (checkQuery.exec()) {
        bool hasRoleColumn = false;
        while (checkQuery.next()) {
            QString columnName = checkQuery.value(1).toString();
            if (columnName == "role") {
                hasRoleColumn = true;
                break;
            }
        }
        if (!hasRoleColumn) {
            // Add role column to existing table
            QSqlQuery alterQuery(m_db);
            if (!alterQuery.exec("ALTER TABLE users ADD COLUMN role INTEGER DEFAULT 0")) {
                qWarning() << "Failed to add role column:" << alterQuery.lastError().text();
                return false;
            }
        }
    }
    // seats table for per-seat tracking (created per ticket_id)
    const QString seatsSql = R"(
    CREATE TABLE IF NOT EXISTS seats (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ticket_id INTEGER,
        username TEXT,
        user_fullname TEXT,
        user_phonenumber TEXT,
        user_email TEXT,
        row INTEGER,
        col INTEGER,
        label TEXT,
        status INTEGER DEFAULT 0
    )
)";
    if (!q.exec(seatsSql)) {
        qWarning() << "Failed to create seats table:" << q.lastError().text();
        return false;
    }
    
    // Check if all required columns exist in seats table, if not add them
    QSqlQuery seatsCheckQuery(m_db);
    seatsCheckQuery.prepare("PRAGMA table_info(seats)");
    if (seatsCheckQuery.exec()) {
        QSet<QString> existingColumns;
        while (seatsCheckQuery.next()) {
            QString columnName = seatsCheckQuery.value(1).toString();
            existingColumns.insert(columnName);
        }
        
        // List of required columns for seats table
        QStringList requiredColumns = {"username", "user_fullname", "user_phonenumber", "user_email"};
        
        for (const QString& column : requiredColumns) {
            if (!existingColumns.contains(column)) {
                // Add missing column to existing seats table
                QSqlQuery alterSeatsQuery(m_db);
                QString alterSql = QString("ALTER TABLE seats ADD COLUMN %1 TEXT").arg(column);
                if (!alterSeatsQuery.exec(alterSql)) {
                    qWarning() << "Failed to add" << column << "column to seats table:" << alterSeatsQuery.lastError().text();
                    return false;
                }
                qDebug() << "Added missing column" << column << "to seats table";
            }
        }
    }
    
    // Check and migrate movie detail columns in tickets table
    QSqlQuery ticketsCheckQuery(m_db);
    ticketsCheckQuery.prepare("PRAGMA table_info(tickets)");
    if (ticketsCheckQuery.exec()) {
        QSet<QString> existingColumns;
        while (ticketsCheckQuery.next()) {
            QString columnName = ticketsCheckQuery.value(1).toString();
            existingColumns.insert(columnName);
        }
        
        // Check if we have the old separate columns
        bool hasOldColumns = existingColumns.contains("description") || 
                           existingColumns.contains("director") || 
                           existingColumns.contains("actors") || 
                           existingColumns.contains("poster") || 
                           existingColumns.contains("genre") || 
                           existingColumns.contains("rating");
        
        if (hasOldColumns && !existingColumns.contains("movie_details")) {
            // Migrate old columns to new movie_details column
            qDebug() << "Migrating movie detail columns to movie_details...";
            
            // First, add the new movie_details column
            QSqlQuery addColumnQuery(m_db);
            if (!addColumnQuery.exec("ALTER TABLE tickets ADD COLUMN movie_details TEXT")) {
                qWarning() << "Failed to add movie_details column:" << addColumnQuery.lastError().text();
                return false;
            }
            
            // Migrate existing data
            QSqlQuery migrateQuery(m_db);
            QString selectColumns;
            if (existingColumns.contains("description")) selectColumns += "description";
            if (existingColumns.contains("director")) {
                if (!selectColumns.isEmpty()) selectColumns += " || '\n导演: ' || ";
                else selectColumns += "'导演: ' || ";
                selectColumns += "director";
            }
            if (existingColumns.contains("actors")) {
                if (!selectColumns.isEmpty()) selectColumns += " || '\n主演: ' || ";
                else selectColumns += "'主演: ' || ";
                selectColumns += "actors";
            }
            if (existingColumns.contains("genre")) {
                if (!selectColumns.isEmpty()) selectColumns += " || '\n类型: ' || ";
                else selectColumns += "'类型: ' || ";
                selectColumns += "genre";
            }
            if (existingColumns.contains("rating") && existingColumns.contains("rating")) {
                if (!selectColumns.isEmpty()) selectColumns += " || '\n评分: ' || CAST(rating AS TEXT)";
                else selectColumns += "'评分: ' || CAST(rating AS TEXT)";
            }
            
            if (!selectColumns.isEmpty()) {
                QString migrateSql = QString("UPDATE tickets SET movie_details = (%1)").arg(selectColumns);
                if (!migrateQuery.exec(migrateSql)) {
                    qWarning() << "Failed to migrate movie details:" << migrateQuery.lastError().text();
                    // Continue anyway, as the column was added
                }
            }
            
            // Drop old columns
            QStringList oldColumns = {"description", "director", "actors", "poster", "genre", "rating"};
            for (const QString& column : oldColumns) {
                if (existingColumns.contains(column)) {
                    QSqlQuery dropQuery(m_db);
                    QString dropSql = QString("ALTER TABLE tickets DROP COLUMN %1").arg(column);
                    if (!dropQuery.exec(dropSql)) {
                        qWarning() << "Failed to drop column" << column << ":" << dropQuery.lastError().text();
                        // Continue anyway
                    } else {
                        qDebug() << "Dropped old column" << column;
                    }
                }
            }
        } else if (!existingColumns.contains("movie_details")) {
            // Add movie_details column if it doesn't exist
            QSqlQuery addColumnQuery(m_db);
            if (!addColumnQuery.exec("ALTER TABLE tickets ADD COLUMN movie_details TEXT")) {
                qWarning() << "Failed to add movie_details column:" << addColumnQuery.lastError().text();
                return false;
            }
            qDebug() << "Added movie_details column to tickets table";
        }
    }
    
    return true;
}
