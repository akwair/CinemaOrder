#include "registerdialog.h"
#include "../auth/authmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

RegisterDialog::RegisterDialog(AuthManager &auth, QWidget *parent)
    : QDialog(parent), m_auth(auth)
{
    setWindowTitle("注册");
    auto *lay = new QVBoxLayout(this);
    auto *userRow = new QHBoxLayout();
    userRow->addWidget(new QLabel("用户名:"));
    m_user = new QLineEdit(); userRow->addWidget(m_user);
    lay->addLayout(userRow);
    auto *passRow = new QHBoxLayout();
    passRow->addWidget(new QLabel("密码:"));
    m_pass = new QLineEdit(); m_pass->setEchoMode(QLineEdit::Password); passRow->addWidget(m_pass);
    lay->addLayout(passRow);

    auto *btn = new QPushButton("注册");
    lay->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
}

void RegisterDialog::onRegisterClicked()
{
    QString u = m_user->text().trimmed();
    QString p = m_pass->text();
    if (u.isEmpty() || p.isEmpty()) { QMessageBox::warning(this, "错误", "请输入用户名和密码"); return; }
    if (m_auth.registerUser(u, p)) { QMessageBox::information(this, "成功", "注册成功，请登录"); accept(); }
    else QMessageBox::warning(this, "失败", "注册失败（可能用户名已存在）");
}
