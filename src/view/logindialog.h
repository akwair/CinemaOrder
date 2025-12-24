#pragma once
#include <QDialog>

class AuthManager;

// 用户登录对话框
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(AuthManager &auth, QWidget *parent = nullptr);
    // 是否认证成功
    bool isAuthenticated() const { return m_ok; }
    // 获取用户角色(0普通 1管理员)
    int getRole() const { return m_role; }
    // 获取登录用户名
    QString getUsername() const { return m_username; }
private slots:
    // 登录按钮点击处理
    void onLogin();
    // 注册按钮点击处理
    void onRegister();
private:
    AuthManager &m_auth;
    bool m_ok = false;          // 认证标志
    int m_role = 0;             // 用户角色
    QString m_username;         // 用户名
    class QLineEdit *m_user;    // 用户名输入框
    class QLineEdit *m_pass;    // 密码输入框
};
