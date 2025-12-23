#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>


class UserInfoDialog : public QDialog {
    Q_OBJECT
public:
    explicit UserInfoDialog(QWidget *parent = nullptr);
    ~UserInfoDialog();
    QString getFullName() const;
    QString getPhoneNumber() const;
    QString getEmail() const;
    
private slots:
    void onAccept();
    void onReject();

private:
    QLineEdit *m_nameEdit;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_emailEdit;
};