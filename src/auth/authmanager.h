#pragma once
#include <QString>

class Database;

// 用户认证管理类
class AuthManager {
public:
    explicit AuthManager(Database &db);
    // 注册新用户
    bool registerUser(const QString &username, const QString &password, int role = 0);
    // 用户登录验证
    bool login(const QString &username, const QString &password, int &role);
    // 判断是否为管理员(role==1)
    bool isAdmin(int role) const { return role == 1; }
private:
    Database &m_db;
    // 密码哈希处理
    QString hashPassword(const QString &password) const;
};
