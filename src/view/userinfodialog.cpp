#include "userinfodialog.h"

UserInfoDialog::UserInfoDialog(QWidget *parent): QDialog(parent){
    m_nameEdit = new QLineEdit(this);
    m_emailEdit=new QLineEdit(this);
    m_phoneEdit=new QLineEdit(this);
    setWindowTitle("用户信息登记");
    setMinimumSize(300,200);
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(new QLabel("姓名:",this));
    mainLay->addWidget(m_nameEdit);
    mainLay->addWidget(new QLabel("电话号码:",this));
    mainLay->addWidget(m_phoneEdit);
    mainLay->addWidget(new QLabel("电子邮箱:",this));
    mainLay->addWidget(m_emailEdit);
    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &UserInfoDialog::onAccept);
    connect(box, &QDialogButtonBox::rejected, this, &UserInfoDialog::reject);
    mainLay->addWidget(box);
    setLayout(mainLay);
}

UserInfoDialog::~UserInfoDialog(){
    
}

void UserInfoDialog::onAccept(){
    if(m_nameEdit->text().isEmpty()||m_phoneEdit->text().isEmpty()||m_emailEdit->text().isEmpty()){
        QMessageBox::warning(this,"输入错误","请填写所有信息！");
        return;
    }
    accept();
}

void UserInfoDialog::onReject(){
    reject();
}

QString UserInfoDialog::getFullName() const{
    return m_nameEdit->text();
}

QString UserInfoDialog::getPhoneNumber() const{
    return m_phoneEdit->text();
}

QString UserInfoDialog::getEmail() const{
    return m_emailEdit->text();
}