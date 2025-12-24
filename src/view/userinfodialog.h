#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>

// 用户信息输入对话框(购票者信息)
class UserInfoDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserInfoDialog(QWidget *parent = nullptr);
    // 获取姓名
    QString getFullName() const { return m_fullName->text(); }
    // 获取电话
    QString getPhoneNumber() const { return m_phoneNumber->text(); }
    // 获取邮箱
    QString getEmail() const { return m_email->text(); }
private:
    QLineEdit *m_fullName;      // 姓名输入框
    QLineEdit *m_phoneNumber;   // 电话输入框
    QLineEdit *m_email;         // 邮箱输入框
};
