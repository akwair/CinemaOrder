#include "logindialog.h"
#include "../auth/authmanager.h"
#include "registerdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

// 构造函数：创建登录界面
LoginDialog::LoginDialog(AuthManager &auth, QWidget *parent)
    : QDialog(parent), m_auth(auth)
{
    setWindowTitle("登录");
    resize(500, 250);
    auto *lay = new QVBoxLayout(this);
    // 用户名行
    auto *userRow = new QHBoxLayout();
    userRow->addWidget(new QLabel("用户名:"));
    m_user = new QLineEdit(); userRow->addWidget(m_user);
    lay->addLayout(userRow);
    // 密码行
    auto *passRow = new QHBoxLayout();
    passRow->addWidget(new QLabel("密  码:"));
    m_pass = new QLineEdit(); m_pass->setEchoMode(QLineEdit::Password); passRow->addWidget(m_pass);
    lay->addLayout(passRow);

    // 按钮行
    auto *btnRow = new QHBoxLayout();
    auto *btnLogin = new QPushButton("登录");
    auto *btnReg = new QPushButton("注册");
    btnRow->addWidget(btnLogin);
    btnRow->addWidget(btnReg);
    lay->addLayout(btnRow);

    // 连接信号槽
    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::onLogin);
    connect(btnReg, &QPushButton::clicked, this, &LoginDialog::onRegister);
}

// 登录按钮处理
void LoginDialog::onLogin()
{
    QString u = m_user->text().trimmed();
    QString p = m_pass->text();
    if (u.isEmpty() || p.isEmpty()) { QMessageBox::warning(this, "错误", "请输入用户名和密码"); return; }
    // 验证登录
    if (m_auth.login(u, p, m_role)) { 
        m_ok = true; 
        m_username = u;
        accept(); 
    }
    else QMessageBox::warning(this, "登录失败", "用户名或密码错误");
}

// 注册按钮处理
void LoginDialog::onRegister()
{
    // 显示注册对话框
    RegisterDialog dlg(m_auth, this);
    dlg.exec();
}
