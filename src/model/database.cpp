#include "database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>

// 构造函数：初始化数据库路径
Database::Database(const QString &path)
    : m_path(path)
{
}

// 析构函数：关闭数据库连接
Database::~Database()
{
    if (m_db.isOpen()) m_db.close();
}

// 打开数据库连接
bool Database::open()
{
    // 检查或创建数据库连接
    if (QSqlDatabase::contains("cinema_connection"))
        m_db = QSqlDatabase::database("cinema_connection");
    else
        m_db = QSqlDatabase::addDatabase("QSQLITE", "cinema_connection");

    m_db.setDatabaseName(m_path);
    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }
    // 确保数据表存在
    return ensureTables();
}

QSqlDatabase Database::db() const
{
    return m_db;
}

// 初始化数据表：创建 tickets、users、seats表
bool Database::ensureTables()
{
    QSqlQuery q(m_db);
    
    // 启用外键约束支持
    if (!q.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:" << q.lastError().text();
        return false;
    }
    
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
    // 用户表
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
    
    // 检查角色列是否存在
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
            // 为现有表添加角色列
            QSqlQuery alterQuery(m_db);
            if (!alterQuery.exec("ALTER TABLE users ADD COLUMN role INTEGER DEFAULT 0")) {
                qWarning() << "Failed to add role column:" << alterQuery.lastError().text();
                return false;
            }
        }
    }
    // 座位表
    const QString seatsSql = R"(
    CREATE TABLE IF NOT EXISTS seats (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ticket_id INTEGER NOT NULL,
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
    
    // 检查座位表字段
    QSqlQuery seatsCheckQuery(m_db);
    seatsCheckQuery.prepare("PRAGMA table_info(seats)");
    if (seatsCheckQuery.exec()) {
        QSet<QString> existingColumns;
        while (seatsCheckQuery.next()) {
            QString columnName = seatsCheckQuery.value(1).toString();
            existingColumns.insert(columnName);
        }
        
        // 座位表字段列表
        QStringList requiredColumns = {"username", "user_fullname", "user_phonenumber", "user_email"};
        
        for (const QString& column : requiredColumns) {
            if (!existingColumns.contains(column)) {
                // 为座位表添加缺失列
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
    
    // 迁移电影详情列
    QSqlQuery ticketsCheckQuery(m_db);
    ticketsCheckQuery.prepare("PRAGMA table_info(tickets)");
    if (ticketsCheckQuery.exec()) {
        QSet<QString> existingColumns;
        while (ticketsCheckQuery.next()) {
            QString columnName = ticketsCheckQuery.value(1).toString();
            existingColumns.insert(columnName);
        }
        
        // 检查旧列
        bool hasOldColumns = existingColumns.contains("description") || 
                           existingColumns.contains("director") || 
                           existingColumns.contains("actors") || 
                           existingColumns.contains("poster") || 
                           existingColumns.contains("genre") || 
                           existingColumns.contains("rating");
        
        if (hasOldColumns && !existingColumns.contains("movie_details")) {
            // 迁移旧列到新movie_details列
            qDebug() << "Migrating movie detail columns to movie_details...";
            
            // 添加新movie_details列
            QSqlQuery addColumnQuery(m_db);
            if (!addColumnQuery.exec("ALTER TABLE tickets ADD COLUMN movie_details TEXT")) {
                qWarning() << "Failed to add movie_details column:" << addColumnQuery.lastError().text();
                return false;
            }
            
            // 迁移现有数据
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
                    // 继续，列已添加
                }
            }
            
            // 删除旧列
            QStringList oldColumns = {"description", "director", "actors", "poster", "genre", "rating"};
            for (const QString& column : oldColumns) {
                if (existingColumns.contains(column)) {
                    QSqlQuery dropQuery(m_db);
                    QString dropSql = QString("ALTER TABLE tickets DROP COLUMN %1").arg(column);
                    if (!dropQuery.exec(dropSql)) {
                        qWarning() << "Failed to drop column" << column << ":" << dropQuery.lastError().text();
                        // 继续
                    } else {
                        qDebug() << "Dropped old column" << column;
                    }
                }
            }
        } else if (!existingColumns.contains("movie_details")) {
            // 添加movie_details列
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
