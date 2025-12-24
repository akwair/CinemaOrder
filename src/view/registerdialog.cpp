#include "registerdialog.h"
#include "../auth/authmanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>

// 构造函数：创建注册界面
RegisterDialog::RegisterDialog(AuthManager &auth, QWidget *parent)
    : QDialog(parent), m_auth(auth)
{
    setWindowTitle("注册");
    auto *lay = new QVBoxLayout(this);
    // 用户名行
    auto *userRow = new QHBoxLayout();
    userRow->addWidget(new QLabel("用户名:"));
    m_user = new QLineEdit(); userRow->addWidget(m_user);
    lay->addLayout(userRow);
    // 密码行
    auto *passRow = new QHBoxLayout();
    passRow->addWidget(new QLabel("密码:"));
    m_pass = new QLineEdit(); m_pass->setEchoMode(QLineEdit::Password); passRow->addWidget(m_pass);
    lay->addLayout(passRow);

    // 管理员勾选框
    m_adminCheck = new QCheckBox("注册为管理员");
    lay->addWidget(m_adminCheck);

    // 管理员校验码行
    auto *codeRow = new QHBoxLayout();
    codeRow->addWidget(new QLabel("管理员校验码:"));
    m_adminCode = new QLineEdit(); m_adminCode->setEchoMode(QLineEdit::Password);
    codeRow->addWidget(m_adminCode);
    lay->addLayout(codeRow);

    // 注册按钮
    auto *btn = new QPushButton("注册");
    lay->addWidget(btn);
    connect(btn, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
}

// 注册按钮处理
void RegisterDialog::onRegisterClicked()
{
    QString u = m_user->text().trimmed();
    QString p = m_pass->text();
    if (u.isEmpty() || p.isEmpty()) { QMessageBox::warning(this, "错误", "请输入用户名和密码"); return; }
    
    int role = 0;
    // 如果选择管理员，需要验证校验码
    if (m_adminCheck->isChecked()) {
        QString code = m_adminCode->text().trimmed();
        if (code != "701001") {  // 校验码设置处
            QMessageBox::warning(this, "错误", "管理员校验码错误");
            return;
        }
        role = 1;  // 1表示管理员
    }
    
    // 调用认证管理器注册
    if (m_auth.registerUser(u, p, role)) { 
        QMessageBox::information(this, "成功", "注册成功，请登录"); 
        accept(); 
    }
    else QMessageBox::warning(this, "失败", "注册失败（可能用户名已存在）");
}
