#pragma once
#include <QtSql/QSqlDatabase>
#include <QString>

class Database {
public:
    explicit Database(const QString &path = QStringLiteral("tickets.db"));
    ~Database();
    bool open();
    QSqlDatabase db() const;
    bool ensureTables();
private:
    QString m_path;
    QSqlDatabase m_db;
};
