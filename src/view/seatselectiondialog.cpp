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


SeatSelectionDialog::SeatSelectionDialog(Database &db, int ticketId, int capacity, int flag, const QString &username, QWidget *parent)
    : QDialog(parent), m_db(db), m_ticketId(ticketId), m_flag(flag), m_username(username)
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

    // compute layout: try 10 rows
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
                btn->installEventFilter(this); // Enable hover events
            }
            else if (s.status==0&&flag==1) {
                btn->setEnabled(false);
                btn->setStyleSheet("background: #c0c0c0; color: #fff;");
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
        if (count > 0) return; // already created
    }
    qDebug() << "Creating seats for ticket" << ticketId;
    // create seats rows=10
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
            } else {
                qDebug() << "Created seat:" << label;
            }
            ins.finish();
            ++created;
        }
    }
    qDebug() << "Created" << created << "seats";
}

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
        qDebug() << "Loaded seat:" << s.label << ", status:" << s.status;
    }
    qDebug() << "Total loaded seats:" << m_seats.size();
}

void SeatSelectionDialog::toggleSeat()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int idx = btn->property("seat_index").toInt();
    if (idx < 0 || idx >= m_seats.size()) return;

    if (btn->isChecked()) {
        // Only show user info dialog for selling tickets (flag == 0)
        if (m_flag == 0) {
            UserInfoDialog userInfoDlg(this);
            if (userInfoDlg.exec() != QDialog::Accepted) {
                btn->setChecked(false);
                return; //用户取消
            }
            // Store user info in the seat info for later use
            m_seats[idx].userFullName = userInfoDlg.getFullName();
            m_seats[idx].userPhoneNumber = userInfoDlg.getPhoneNumber();
            m_seats[idx].userEmail = userInfoDlg.getEmail();
        }
        m_selected.append(m_seats[idx]);
        btn->setStyleSheet("background: #6aa84f; color: #fff;");
    } else {
        // remove from selected
        for (int i = 0; i < m_selected.size(); ++i) {
            if (m_selected[i].id == m_seats[idx].id) { m_selected.remove(i); break; }
        }
        btn->setStyleSheet("background: #e8f0ff; color: #000;");
    }
}

bool SeatSelectionDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        QPushButton *btn = qobject_cast<QPushButton*>(obj);
        if (btn) {
            int idx = btn->property("seat_index").toInt();
            if (idx >= 0 && idx < m_seats.size()) {
                const SeatInfo &s = m_seats[idx];
                if (s.status == 1) { // Only show info for sold seats
                    QString info = QString("座位: %1\n购买者: %2\n电话: %3\n邮箱: %4")
                                   .arg(s.label, s.userFullName, s.userPhoneNumber, s.userEmail);
                    btn->setToolTip(info);
                }
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

