#include "ticketformdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>

TicketFormDialog::TicketFormDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("新增/编辑场次");
    auto *lay = new QVBoxLayout(this);
    m_movie = new QLineEdit();
    m_cinema = new QLineEdit();
    m_date = new QLineEdit();
    m_time = new QLineEdit();
    m_duration = new QSpinBox(); m_duration->setRange(1, 10000); m_duration->setValue(120);
    m_price = new QDoubleSpinBox(); m_price->setRange(0, 100000); m_price->setDecimals(2); m_price->setValue(60.0);
    m_hall = new QLineEdit();
    m_capacity = new QSpinBox(); m_capacity->setRange(1, 100000); m_capacity->setValue(100);

    auto addRow = [&](const QString &label, QWidget *w){
        auto *h = new QHBoxLayout();
        h->addWidget(new QLabel(label));
        h->addWidget(w);
        lay->addLayout(h);
    };

    addRow("电影名称:", m_movie);
    addRow("影院名称:", m_cinema);
    addRow("放映日期 (YYYY-MM-DD):", m_date);
    addRow("放映时间 (HH:MM):", m_time);
    addRow("电影时长(分钟):", m_duration);
    addRow("票价:", m_price);
    addRow("放映厅:", m_hall);
    addRow("座位容量:", m_capacity);

    auto *btnRow = new QHBoxLayout();
    auto *ok = new QPushButton("确定");
    auto *cancel = new QPushButton("取消");
    btnRow->addStretch(); btnRow->addWidget(ok); btnRow->addWidget(cancel);
    lay->addLayout(btnRow);

    connect(ok, &QPushButton::clicked, this, &TicketFormDialog::onAccept);
    connect(cancel, &QPushButton::clicked, this, &TicketFormDialog::reject);
}

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
}

Ticket TicketFormDialog::ticket() const
{
    return m_ticket;
}

void TicketFormDialog::onAccept()
{
    // validation
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
    accept();
}


