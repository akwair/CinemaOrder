#include "ticketformdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QTextEdit>

// 构造函数：创建新增/编辑表单
TicketFormDialog::TicketFormDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("新增/编辑场次");
    auto *lay = new QVBoxLayout(this);
    
    // 基本信息组
    QGroupBox *basicGroup = new QGroupBox("基本信息", this);
    QFormLayout *basicLayout = new QFormLayout(basicGroup);
    
    m_movie = new QLineEdit();
    basicLayout->addRow("电影名称:", m_movie);
    
    m_cinema = new QLineEdit();
    basicLayout->addRow("影院名称:", m_cinema);
    
    m_date = new QLineEdit();
    basicLayout->addRow("放映日期 (YYYY-MM-DD):", m_date);
    
    m_time = new QLineEdit();
    basicLayout->addRow("放映时间 (HH:MM):", m_time);
    
    m_duration = new QSpinBox(); 
    m_duration->setRange(1, 10000); 
    m_duration->setValue(120);
    basicLayout->addRow("电影时长(分钟):", m_duration);
    
    m_price = new QDoubleSpinBox(); 
    m_price->setRange(0, 100000); 
    m_price->setDecimals(2); 
    m_price->setValue(60.0);
    basicLayout->addRow("票价:", m_price);
    
    m_hall = new QLineEdit();
    basicLayout->addRow("放映厅:", m_hall);
    
    m_capacity = new QSpinBox(); 
    m_capacity->setRange(1, 100000); 
    m_capacity->setValue(100);
    basicLayout->addRow("座位容量:", m_capacity);
    
    lay->addWidget(basicGroup);
    
    // 电影详情组
    QGroupBox *detailGroup = new QGroupBox("电影详情", this);
    QFormLayout *detailLayout = new QFormLayout(detailGroup);
    
    m_director = new QLineEdit();
    detailLayout->addRow("导演:", m_director);
    
    m_actors = new QLineEdit();
    detailLayout->addRow("主演:", m_actors);
    
    m_genre = new QLineEdit();
    detailLayout->addRow("类型:", m_genre);
    
    m_rating = new QDoubleSpinBox();
    m_rating->setRange(0.0, 10.0);
    m_rating->setSingleStep(0.1);
    m_rating->setDecimals(1);
    detailLayout->addRow("评分:", m_rating);
    
    m_poster = new QLineEdit();
    m_poster->setPlaceholderText("海报文件路径...");
    detailLayout->addRow("海报:", m_poster);
    
    m_description = new QTextEdit();
    m_description->setMaximumHeight(80);
    m_description->setPlaceholderText("输入电影剧情简介...");
    detailLayout->addRow("剧情简介:", m_description);
    
    lay->addWidget(detailGroup);

    auto *btnRow = new QHBoxLayout();
    auto *ok = new QPushButton("确定");
    auto *cancel = new QPushButton("取消");
    btnRow->addStretch(); btnRow->addWidget(ok); btnRow->addWidget(cancel);
    lay->addLayout(btnRow);

    connect(ok, &QPushButton::clicked, this, &TicketFormDialog::onAccept);
    connect(cancel, &QPushButton::clicked, this, &TicketFormDialog::reject);
}

// 加载票务到表单控件
void TicketFormDialog::setTicket(const Ticket &t)
{
    m_ticket = t;
    m_movie->setText(t.movieName);
    m_cinema->setText(t.cinemaName);
    m_date->setText(t.showDate);
    m_time->setText(t.showTime);
    m_duration->setValue(t.duration);
    m_price->setValue(t.price);
    m_hall->setText(t.hall);
    m_capacity->setValue(t.capacity);
    
    // 设置电影详情（合并字段）
    m_description->setPlainText(t.movieDetails);
    // 其它细分字段保持为空（若需要可在此解析并填充）
    m_director->clear();
    m_actors->clear();
    m_genre->clear();
    m_rating->setValue(0.0);
    m_poster->clear();
}

Ticket TicketFormDialog::ticket() const
{
    return m_ticket;
}

// 确定：验证并保存数据
void TicketFormDialog::onAccept()
{
    // 数据验证
    if (m_movie->text().trimmed().isEmpty()) { QMessageBox::warning(this, "验证失败", "电影名称不能为空"); return; }
    if (m_cinema->text().trimmed().isEmpty()) { QMessageBox::warning(this, "验证失败", "影院名称不能为空"); return; }
    m_ticket.movieName = m_movie->text().trimmed();
    m_ticket.cinemaName = m_cinema->text().trimmed();
    m_ticket.showDate = m_date->text().trimmed();
    m_ticket.showTime = m_time->text().trimmed();
    m_ticket.duration = m_duration->value();
    m_ticket.price = m_price->value();
    m_ticket.hall = m_hall->text().trimmed();
    m_ticket.capacity = m_capacity->value();
    
    // 保存电影详情（合并字段）
    m_ticket.movieDetails = m_description->toPlainText().trimmed();
    
    accept();
}


