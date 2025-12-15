#pragma once
#include <QDialog>
#include <QVector>

class Database;

struct SeatInfo {
    int id; // seats table id, 0 if not persisted yet
    int row;
    int col;
    QString label;
    int status; // 0 available, 1 sold
};

class SeatSelectionDialog : public QDialog {
    Q_OBJECT
public:
    SeatSelectionDialog(Database &db, int ticketId, int capacity, QWidget *parent = nullptr);
    QVector<SeatInfo> selectedSeats() const { return m_selected; }
private slots:
    void toggleSeat();
private:
    void ensureSeatsCreated(int ticketId, int capacity);
    void loadSeats(int ticketId);
    Database &m_db;
    int m_ticketId;
    QVector<SeatInfo> m_seats;
    QVector<SeatInfo> m_selected;
};
