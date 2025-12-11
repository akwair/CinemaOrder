#pragma once
#include <QString>
#include <QTextStream>

class TicketController;

class ConsoleView {
public:
    explicit ConsoleView(TicketController &controller);
    void run();
private:
    TicketController &m_ctrl;
    QTextStream m_in;
    QTextStream m_out;
    QString readLine();
    void showTicket(const class Ticket &t);
};
