#include "authmanager.h"
#include "../model/database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QCryptographicHash>
#include <QDebug>

// 构造函数：传入数据库引用
AuthManager::AuthManager(Database &db)
    : m_db(db)
{
}

// 对密码进行SHA256哈希处理
QString AuthManager::hashPassword(const QString &password) const
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

// 注册新用户
bool AuthManager::registerUser(const QString &username, const QString &password, int role)
{
    QSqlQuery q(m_db.db());
    // 插入用户：用户名、密码哈希、角色
    q.prepare("INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)");
    q.addBindValue(username);
    q.addBindValue(hashPassword(password));
    q.addBindValue(role);
    if (!q.exec()) {
        qWarning() << "Register failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// 登录验证
bool AuthManager::login(const QString &username, const QString &password, int &role)
{
    QSqlQuery q(m_db.db());
    // 查询用户名对应的密码哈希和角色
    q.prepare("SELECT password_hash, role FROM users WHERE username = ?");
    q.addBindValue(username);
    if (!q.exec()) {
        qWarning() << "Login query failed:" << q.lastError().text();
        return false;
    }
    if (!q.next()) return false;
    // 比对密码哈希
    QString stored = q.value(0).toString();
    if (stored != hashPassword(password)) return false;
    // 获取用户角色
    role = q.value(1).toInt();
    return true;
}
