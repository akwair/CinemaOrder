#pragma once
#include <QDialog>

class AuthManager;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(AuthManager &auth, QWidget *parent = nullptr);
    bool isAuthenticated() const { return m_ok; }
private slots:
    void onLogin();
    void onRegister();
private:
    AuthManager &m_auth;
    bool m_ok = false;
    class QLineEdit *m_user;
    class QLineEdit *m_pass;
};
