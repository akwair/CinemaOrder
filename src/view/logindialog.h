#pragma once
#include <QDialog>

class AuthManager;

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(AuthManager &auth, QWidget *parent = nullptr);
    bool isAuthenticated() const { return m_ok; }
    int getRole() const { return m_role; }
    QString getUsername() const { return m_username; }
private slots:
    void onLogin();
    void onRegister();
private:
    AuthManager &m_auth;
    bool m_ok = false;
    int m_role = 0;
    QString m_username;
    class QLineEdit *m_user;
    class QLineEdit *m_pass;
};
