#pragma once
#include <QString>

class Database;

class AuthManager {
public:
    explicit AuthManager(Database &db);
    bool registerUser(const QString &username, const QString &password, int role = 0);
    bool login(const QString &username, const QString &password, int &role);
    bool isAdmin(int role) const { return role == 1; }
private:
    Database &m_db;
    QString hashPassword(const QString &password) const;
};
