#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include "../model/ticket.h"

// 票务新增/编辑表单对话框
class TicketFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit TicketFormDialog(QWidget *parent = nullptr);
    // 设置编辑的票务信息
    void setTicket(const Ticket &t);
    // 获取表单输入的票务对象
    Ticket ticket() const;
private slots:
    // 确定按钮处理
    void onAccept();
private:
    Ticket m_ticket;                // 票务对象
    QLineEdit *m_movie;             // 电影名称
    QLineEdit *m_cinema;            // 影院名称
    QLineEdit *m_date;              // 放映日期
    QLineEdit *m_time;              // 放映时间
    QSpinBox *m_duration;           // 时长(分钟)
    QDoubleSpinBox *m_price;        // 票价
    QLineEdit *m_hall;              // 放映厅
    QSpinBox *m_capacity;           // 座位容量
    
    // 电影详情字段
    QTextEdit *m_description;       // 剧情简介
    QLineEdit *m_director;          // 导演
    QLineEdit *m_actors;            // 主演
    QLineEdit *m_genre;             // 电影类型
    QDoubleSpinBox *m_rating;       // 评分
};
