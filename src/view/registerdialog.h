#pragma once
#include <QDialog>

class AuthManager;

// 用户注册对话框
class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(AuthManager &auth, QWidget *parent = nullptr);
private slots:
    // 注册按钮点击处理
    void onRegisterClicked();
private:
    AuthManager &m_auth;
    class QLineEdit *m_user;         // 用户名输入框
    class QLineEdit *m_pass;         // 密码输入框
    class QCheckBox *m_adminCheck;   // 管理员勾选框
    class QLineEdit *m_adminCode;    // 管理员校验码
};
