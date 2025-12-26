#include "seatselectiondialog.h"
#include "../model/database.h"
#include "userinfodialog.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDebug>
#include <QEvent>


// 构造函数：创建座位选择对话框（flag:0售票 1退票）
SeatSelectionDialog::SeatSelectionDialog(Database &db, int ticketId, int capacity, int flag, const QString &username, bool isAdmin, QWidget *parent)
    : QDialog(parent), m_db(db), m_ticketId(ticketId), m_flag(flag), m_username(username), m_isAdmin(isAdmin)
{
    setWindowTitle(tr("选择座位"));
    qDebug() << "SeatSelectionDialog: ticketId=" << ticketId << ", capacity=" << capacity << ", flag=" << flag;
    ensureSeatsCreated(ticketId, capacity);
    loadSeats(ticketId);
    qDebug() << "Loaded" << m_seats.size() << "seats";

    QVBoxLayout *mainLay = new QVBoxLayout(this);
    QScrollArea *scroll = new QScrollArea(this);
    QWidget *gridWidget = new QWidget(this);
    QGridLayout *grid = new QGridLayout(gridWidget);

    // 计算位置：10行
    int rows = 10;
    int cols = (m_seats.size() + rows - 1) / rows;
    int idx = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            if (idx >= m_seats.size()) break;
            auto &s = m_seats[idx];
            QPushButton *btn = new QPushButton(s.label, this);
            btn->setProperty("seat_index", idx);
            if (s.status == 1 && flag == 0) {
                btn->setEnabled(false);
                btn->setStyleSheet("background: #c05050; color: #fff;");
                btn->setToolTip("已售出");
                btn->installEventFilter(this); // 启用悬停事件
            }
            else if (flag == 1) { // 退票模式
                if (s.status == 0) {
                    // 未售出，不可退
                    btn->setEnabled(false);
                    btn->setStyleSheet("background: #c0c0c0; color: #fff;");
                } else {
                    // 已售出，检查权限
                    // 如果是管理员，或者座位属于当前用户，则允许退票
                    if (m_isAdmin || s.username == m_username) {
                        btn->setCheckable(true);
                        btn->setStyleSheet("background: #e8f0ff;");
                        connect(btn, &QPushButton::clicked, this, &SeatSelectionDialog::toggleSeat);
                    } else {
                        // 不属于当前用户且非管理员
                        btn->setEnabled(false);
                        btn->setStyleSheet("background: #c05050; color: #fff;"); // 显示为红色（别人的票）
                        btn->setToolTip("非本人购买，无法退票");
                    }
                }
            }
            else {
                btn->setCheckable(true);
                btn->setStyleSheet("background: #e8f0ff;");
                connect(btn, &QPushButton::clicked, this, &SeatSelectionDialog::toggleSeat);
            }
            grid->addWidget(btn, r, c);
            ++idx;
        }
    }
    gridWidget->setLayout(grid);
    scroll->setWidget(gridWidget);
    scroll->setWidgetResizable(true);
    mainLay->addWidget(scroll);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLay->addWidget(box);
    setLayout(mainLay);
    resize(600, 400);
}

// 初始化座位数据（首次创建）
void SeatSelectionDialog::ensureSeatsCreated(int ticketId, int capacity)
{
    qDebug() << "ensureSeatsCreated: ticketId=" << ticketId << ", capacity=" << capacity;
    QSqlQuery q(m_db.db());
    q.prepare("SELECT COUNT(*) FROM seats WHERE ticket_id = ?");
    q.addBindValue(ticketId);
    if (!q.exec()) {
        qDebug() << "Failed to query seats count:" << q.lastError().text();
        return;
    }
    if (q.next()) {
        int count = q.value(0).toInt();
        qDebug() << "Found" << count << "seats for ticket" << ticketId;
        if (count > 0) return; // 已创建
    }
    qDebug() << "Creating seats for ticket" << ticketId;
    // 创建座位簇
    int rows = 10;
    int cols = (capacity + rows - 1) / rows;
    int created = 0;
    QSqlQuery ins(m_db.db());
    ins.prepare("INSERT INTO seats (ticket_id, row, col, label, status) VALUES (?,?,?,?,0)");
    for (int r = 0; r < rows && created < capacity; ++r) {
        for (int c = 0; c < cols && created < capacity; ++c) {
            QString label = QString(QChar('A' + r)) + QString::number(c + 1);
            ins.addBindValue(ticketId);
            ins.addBindValue(r);
            ins.addBindValue(c);
            ins.addBindValue(label);
            if (!ins.exec()) {
                qWarning() << "insert seat failed:" << ins.lastError().text();
            } 
            ins.finish();
            ++created;
        }
    }
    qDebug() << "Created" << created << "seats";
}

// 加载指定票务的座位信息
void SeatSelectionDialog::loadSeats(int ticketId)
{
    qDebug() << "loadSeats: ticketId=" << ticketId;
    m_seats.clear();
    QSqlQuery q(m_db.db());
    q.prepare("SELECT id,row,col,label,status,username,user_fullname,user_phonenumber,user_email FROM seats WHERE ticket_id = ? ORDER BY row, col");
    q.addBindValue(ticketId);
    if (!q.exec()) {
        qDebug() << "Failed to load seats:" << q.lastError().text();
        return;
    }
    while (q.next()) {
        SeatInfo s;
        s.id = q.value(0).toInt();
        s.row = q.value(1).toInt();
        s.col = q.value(2).toInt();
        s.label = q.value(3).toString();
        s.status = q.value(4).toInt();
        s.username = q.value(5).toString();
        s.userFullName = q.value(6).toString();
        s.userPhoneNumber = q.value(7).toString();
        s.userEmail = q.value(8).toString();
        m_seats.append(s);
    }
}

// 座位按钮：选中/取消
void SeatSelectionDialog::toggleSeat()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int idx = btn->property("seat_index").toInt();
    if (idx < 0 || idx >= m_seats.size()) return;

    if (btn->isChecked()) {
        // 仅在售票时显示用户信息对话框
        if (m_flag == 0) {
            UserInfoDialog userInfoDlg(this);
            if (userInfoDlg.exec() != QDialog::Accepted) {
                btn->setChecked(false);
                return;
            }
            // 保存用户信息到座位
            m_seats[idx].userFullName = userInfoDlg.getFullName();
            m_seats[idx].userPhoneNumber = userInfoDlg.getPhoneNumber();
            m_seats[idx].userEmail = userInfoDlg.getEmail();
        }
        m_selected.append(m_seats[idx]);
        btn->setStyleSheet("background: #6aa84f; color: #fff;");
    } else {
        // 从已选中清除
        for (int i = 0; i < m_selected.size(); ++i) {
            if (m_selected[i].id == m_seats[idx].id) { m_selected.remove(i); break; }
        }
        btn->setStyleSheet("background: #e8f0ff; color: #000;");
    }
}

// 事件过滤：显示座位悬浮提示
bool SeatSelectionDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        QPushButton *btn = qobject_cast<QPushButton*>(obj);
        if (btn) {
            int idx = btn->property("seat_index").toInt();
            if (idx >= 0 && idx < m_seats.size()) {
                const SeatInfo &s = m_seats[idx];
                if (s.status == 1) { // 仅显示已售座位信息
                    // 管理员可以查看所有座位信息，普通用户只能查看自己的座位信息
                    if (m_isAdmin || s.username == m_username) {
                        QString info = QString("座位: %1\n用户名: %2\n购买者: %3\n电话: %4\n邮箱: %5")
                                       .arg(s.label, s.username, s.userFullName, s.userPhoneNumber, s.userEmail);
                        btn->setToolTip(info);
                    } else {
                        // 普通用户查看他人座位，仅显示座位号和已售出状态
                        btn->setToolTip(QString("座位: %1\n状态: 已售出").arg(s.label));
                    }
                }
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

