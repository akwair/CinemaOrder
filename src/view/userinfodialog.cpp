#include "userinfodialog.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

// 构造函数：创建用户信息输入表单
UserInfoDialog::UserInfoDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("购买者信息");
    QFormLayout *form = new QFormLayout(this);
    // 创建输入字段
    m_fullName = new QLineEdit(this);
    m_phoneNumber = new QLineEdit(this);
    m_email = new QLineEdit(this);
    // 正则约束邮箱和电话输入格式
    const QRegularExpression phonePattern("^\\+?\\d{7,15}$");
    m_phoneNumber->setValidator(new QRegularExpressionValidator(phonePattern, this));
    const QRegularExpression emailPattern("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$");
    m_email->setValidator(new QRegularExpressionValidator(emailPattern, this));
    // 设置占位符提示
    m_fullName->setPlaceholderText("姓名");
    m_phoneNumber->setPlaceholderText("电话");
    m_email->setPlaceholderText("邮箱");
    // 添加到表单
    form->addRow("姓名", m_fullName);
    form->addRow("电话", m_phoneNumber);
    form->addRow("邮箱", m_email);

    // 确定和取消按钮
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addRow(box);
}
