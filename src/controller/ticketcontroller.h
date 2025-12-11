#pragma once
#include "../model/ticket.h"
#include "../model/database.h"
#include <QList>
#include <QString>

class TicketController {
public:
    explicit TicketController(Database &db);
    bool addTicket(const Ticket &t);
    QList<Ticket> findTickets(const QString &movie = QString(), const QString &cinema = QString(), const QString &date = QString(), const QString &time = QString());
    QList<Ticket> listAll(const QString &orderBy = QString());
    bool deleteTicket(int id);
    bool sellTickets(int id, int qty);
    bool refundTickets(int id, int qty);
    bool saveToFile(const QString &path);
    bool loadFromFile(const QString &path, bool clearBefore = false);
private:
    Database &m_db;
};

// (No public DB helper here.)
