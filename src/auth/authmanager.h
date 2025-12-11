#pragma once
#include <QString>

class Database;

class AuthManager {
public:
    explicit AuthManager(Database &db);
    bool registerUser(const QString &username, const QString &password);
    bool login(const QString &username, const QString &password);
private:
    Database &m_db;
    QString hashPassword(const QString &password) const;
};
