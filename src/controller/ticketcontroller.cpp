#include "ticketcontroller.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QStringList>
#include <QList>

TicketController::TicketController(Database &db)
    : m_db(db)
{
}

bool TicketController::addTicket(const Ticket &t)
{
    QSqlQuery q(m_db.db());
    q.prepare("INSERT INTO tickets (movieName,cinemaName,showDate,showTime,duration,price,hall,capacity,sold) VALUES (?,?,?,?,?,?,?,?,?)");
    q.addBindValue(t.movieName);
    q.addBindValue(t.cinemaName);
    q.addBindValue(t.showDate);
    q.addBindValue(t.showTime);
    q.addBindValue(t.duration);
    q.addBindValue(t.price);
    q.addBindValue(t.hall);
    q.addBindValue(t.capacity);
    q.addBindValue(t.sold);
    if (!q.exec()) {
        qWarning() << "Insert failed:" << q.lastError().text();
        return false;
    }
    return true;
}

static Ticket rowToTicket(const QSqlQuery &q)
{
    Ticket t;
    t.id = q.value("id").toInt();
    t.movieName = q.value("movieName").toString();
    t.cinemaName = q.value("cinemaName").toString();
    t.showDate = q.value("showDate").toString();
    t.showTime = q.value("showTime").toString();
    t.duration = q.value("duration").toInt();
    t.price = q.value("price").toDouble();
    t.hall = q.value("hall").toString();
    t.capacity = q.value("capacity").toInt();
    t.sold = q.value("sold").toInt();
    return t;
}

QList<Ticket> TicketController::findTickets(const QString &movie, const QString &cinema, const QString &date, const QString &time)
{
    QList<Ticket> list;
    QSqlQuery q(m_db.db());
    QString sql = "SELECT * FROM tickets WHERE 1=1";
    if (!movie.isEmpty()) sql += " AND movieName LIKE '%' || ? || '%'";
    if (!cinema.isEmpty()) sql += " AND cinemaName LIKE '%' || ? || '%'";
    if (!date.isEmpty()) sql += " AND showDate = ?";
    if (!time.isEmpty()) sql += " AND showTime = ?";
    q.prepare(sql);
    if (!movie.isEmpty()) q.addBindValue(movie);
    if (!cinema.isEmpty()) q.addBindValue(cinema);
    if (!date.isEmpty()) q.addBindValue(date);
    if (!time.isEmpty()) q.addBindValue(time);
    if (!q.exec()) {
        qWarning() << "Query failed:" << q.lastError().text();
        return list;
    }
    while (q.next()) list.append(rowToTicket(q));
    return list;
}

QList<Ticket> TicketController::listAll(const QString &orderBy)
{
    QList<Ticket> list;
    QSqlQuery q(m_db.db());
    QString sql = "SELECT * FROM tickets";
    if (!orderBy.isEmpty()) sql += " ORDER BY " + orderBy;
    if (!q.exec(sql)) {
        qWarning() << "List all failed:" << q.lastError().text();
        return list;
    }
    while (q.next()) list.append(rowToTicket(q));
    return list;
}

bool TicketController::deleteTicket(int id)
{
    QSqlQuery q(m_db.db());
    q.prepare("DELETE FROM tickets WHERE id = ?");
    q.addBindValue(id);
    if (!q.exec()) {
        qWarning() << "Delete failed:" << q.lastError().text();
        return false;
    }
    return true;
}

bool TicketController::sellTickets(int id, int qty)
{
    QSqlQuery q(m_db.db());
    q.prepare("UPDATE tickets SET sold = sold + ? WHERE id = ? AND sold + ? <= capacity");
    q.addBindValue(qty);
    q.addBindValue(id);
    q.addBindValue(qty);
    if (!q.exec()) {
        qWarning() << "Sell failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool TicketController::refundTickets(int id, int qty)
{
    QSqlQuery q(m_db.db());
    q.prepare("UPDATE tickets SET sold = sold - ? WHERE id = ? AND sold - ? >= 0");
    q.addBindValue(qty);
    q.addBindValue(id);
    q.addBindValue(qty);
    if (!q.exec()) {
        qWarning() << "Refund failed:" << q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}

bool TicketController::saveToFile(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&f);
    auto list = listAll();
    for (const Ticket &t : list) {
        out << t.id << ','
            << t.movieName << ','
            << t.cinemaName << ','
            << t.showDate << ','
            << t.showTime << ','
            << t.duration << ','
            << t.price << ','
            << t.hall << ','
            << t.capacity << ','
            << t.sold << '\n';
    }
    return true;
}

bool TicketController::loadFromFile(const QString &path, bool clearBefore)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    if (clearBefore) {
        QSqlQuery q(m_db.db());
        q.exec("DELETE FROM tickets");
    }
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        QStringList parts = line.split(',');
        if (parts.size() < 10) continue;
        Ticket t;
        // ignore id from file (autoincrement)
        t.movieName = parts[1];
        t.cinemaName = parts[2];
        t.showDate = parts[3];
        t.showTime = parts[4];
        t.duration = parts[5].toInt();
        t.price = parts[6].toDouble();
        t.hall = parts[7];
        t.capacity = parts[8].toInt();
        t.sold = parts[9].toInt();
        addTicket(t);
    }
    return true;
}
