#pragma once
#include <QDialog>

class AuthManager;

class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(AuthManager &auth, QWidget *parent = nullptr);
private slots:
    void onRegisterClicked();
private:
    AuthManager &m_auth;
    class QLineEdit *m_user;
    class QLineEdit *m_pass;
};
