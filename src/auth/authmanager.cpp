#include "authmanager.h"
#include "../model/database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QCryptographicHash>
#include <QDebug>

AuthManager::AuthManager(Database &db)
    : m_db(db)
{
}

QString AuthManager::hashPassword(const QString &password) const
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool AuthManager::registerUser(const QString &username, const QString &password)
{
    QSqlQuery q(m_db.db());
    q.prepare("INSERT INTO users (username, password_hash) VALUES (?, ?)");
    q.addBindValue(username);
    q.addBindValue(hashPassword(password));
    if (!q.exec()) {
        qWarning() << "Register failed:" << q.lastError().text();
        return false;
    }
    return true;
}

bool AuthManager::login(const QString &username, const QString &password)
{
    QSqlQuery q(m_db.db());
    q.prepare("SELECT password_hash FROM users WHERE username = ?");
    q.addBindValue(username);
    if (!q.exec()) {
        qWarning() << "Login query failed:" << q.lastError().text();
        return false;
    }
    if (!q.next()) return false;
    QString stored = q.value(0).toString();
    return stored == hashPassword(password);
}
