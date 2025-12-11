#include "logindialog.h"
#include "../auth/authmanager.h"
#include "registerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

LoginDialog::LoginDialog(AuthManager &auth, QWidget *parent)
    : QDialog(parent), m_auth(auth)
{
    setWindowTitle("登录");
    resize(500, 250);
    auto *lay = new QVBoxLayout(this);
    auto *userRow = new QHBoxLayout();
    userRow->addWidget(new QLabel("用户名:"));
    m_user = new QLineEdit(); userRow->addWidget(m_user);
    lay->addLayout(userRow);
    auto *passRow = new QHBoxLayout();
    passRow->addWidget(new QLabel("密 码:"));
    m_pass = new QLineEdit(); m_pass->setEchoMode(QLineEdit::Password); passRow->addWidget(m_pass);
    lay->addLayout(passRow);

    auto *btnRow = new QHBoxLayout();
    auto *btnLogin = new QPushButton("登录");
    auto *btnReg = new QPushButton("注册");
    btnRow->addWidget(btnLogin);
    btnRow->addWidget(btnReg);
    lay->addLayout(btnRow);

    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(btnReg, &QPushButton::clicked, this, &LoginDialog::onRegister);
}

void LoginDialog::onLogin()
{
    QString u = m_user->text().trimmed();
    QString p = m_pass->text();
    if (u.isEmpty() || p.isEmpty()) { QMessageBox::warning(this, "错误", "请输入用户名和密码"); return; }
    if (m_auth.login(u, p)) { m_ok = true; accept(); }
    else QMessageBox::warning(this, "登录失败", "用户名或密码错误");
}

void LoginDialog::onRegister()
{
    RegisterDialog dlg(m_auth, this);
    dlg.exec();
}
