#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include "../model/ticket.h"

class TicketFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit TicketFormDialog(QWidget *parent = nullptr);
    void setTicket(const Ticket &t);
    Ticket ticket() const;
private slots:
    void onAccept();
private:
    Ticket m_ticket;
    QLineEdit *m_movie;
    QLineEdit *m_cinema;
    QLineEdit *m_date;
    QLineEdit *m_time;
    QSpinBox *m_duration;
    QDoubleSpinBox *m_price;
    QLineEdit *m_hall;
    QSpinBox *m_capacity;
    
    // 电影详情字段
    QTextEdit *m_description;
    QLineEdit *m_director;
    QLineEdit *m_actors;
    QLineEdit *m_genre;
    QDoubleSpinBox *m_rating;
    QLineEdit *m_poster;
    
    int flag;//分辨是售票还是退票
};
