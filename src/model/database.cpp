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
        sold INTEGER
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
        password_hash TEXT
    )
)";
    if (!q.exec(usersSql)) {
        qWarning() << "Failed to create users table:" << q.lastError().text();
        return false;
    }
    // seats table for per-seat tracking (created per ticket_id)
    const QString seatsSql = R"(
    CREATE TABLE IF NOT EXISTS seats (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        ticket_id INTEGER,
        row INTEGER,
        col INTEGER,
        label TEXT,
        status INTEGER DEFAULT 0,
        FOREIGN KEY(ticket_id) REFERENCES tickets(id)
    )
    )";
    if (!q.exec(seatsSql)) {
        qWarning() << "Failed to create seats table:" << q.lastError().text();
        return false;
    }
    return true;
}
