#include "seatselectiondialog.h"
#include "../model/database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDebug>

SeatSelectionDialog::SeatSelectionDialog(Database &db, int ticketId, int capacity, QWidget *parent)
    : QDialog(parent), m_db(db), m_ticketId(ticketId)
{
    setWindowTitle(tr("选择座位"));
    ensureSeatsCreated(ticketId, capacity);
    loadSeats(ticketId);

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
            if (s.status == 1) {
                btn->setEnabled(false);
                btn->setStyleSheet("background: #c05050; color: #fff;");
            } else {
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
    QSqlQuery q(m_db.db());
    q.prepare("SELECT COUNT(*) FROM seats WHERE ticket_id = ?");
    q.addBindValue(ticketId);
    if (!q.exec()) return;
    if (q.next()) {
        int count = q.value(0).toInt();
        if (count > 0) return; // already created
    }
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
            if (!ins.exec()) qWarning() << "insert seat failed:" << ins.lastError().text();
            ins.finish();
            ++created;
        }
    }
}

void SeatSelectionDialog::loadSeats(int ticketId)
{
    m_seats.clear();
    QSqlQuery q(m_db.db());
    q.prepare("SELECT id,row,col,label,status FROM seats WHERE ticket_id = ? ORDER BY row, col");
    q.addBindValue(ticketId);
    if (!q.exec()) return;
    while (q.next()) {
        SeatInfo s;
        s.id = q.value(0).toInt();
        s.row = q.value(1).toInt();
        s.col = q.value(2).toInt();
        s.label = q.value(3).toString();
        s.status = q.value(4).toInt();
        m_seats.append(s);
    }
}

void SeatSelectionDialog::toggleSeat()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int idx = btn->property("seat_index").toInt();
    if (idx < 0 || idx >= m_seats.size()) return;
    if (btn->isChecked()) {
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

