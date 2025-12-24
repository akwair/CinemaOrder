#pragma once
#include <QDialog>
#include <QVector>
#include "userinfodialog.h"

class Database;

struct SeatInfo {
    int id; // seats table id, 0 if not persisted yet
    int row;
    int col;
    QString label;
    int status; // 0 available, 1 sold
    QString username;
    QString userFullName;
    QString userPhoneNumber;
    QString userEmail;
};

class SeatSelectionDialog : public QDialog {
    Q_OBJECT
public:
    SeatSelectionDialog(Database &db, int ticketId, int capacity, int flag, const QString &username, QWidget *parent = nullptr);//flag 0为售票，1为退票
    QVector<SeatInfo> selectedSeats() const { return m_selected; }
private slots:
    void toggleSeat();
private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void ensureSeatsCreated(int ticketId, int capacity);
    void loadSeats(int ticketId);
    Database &m_db;
    int m_ticketId;
    int m_flag; // 0 for sell, 1 for refund
    QString m_username;
    QVector<SeatInfo> m_seats;
    QVector<SeatInfo> m_selected;
};
