#pragma once
#include <QMainWindow>
class QSqlTableModel;
class TicketController;
class Database;

class UserMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit UserMainWindow(Database &db, TicketController &controller, const QString &username, QWidget *parent = nullptr);
private slots:
    void onBuyTicket();
    void onRefund();
    void onSearch();
    void onSort();
    void onToggleTheme();
    void applyTheme(bool dark);
    void onFadeFinished();
    void onRestore();
    void onViewMyTickets();
    void onViewMovieDetail();
private:
    TicketController &m_ctrl;
    QSqlTableModel *m_model;
    Database &m_db;
    QString m_username;
    void refresh();
    // UI helpers
    bool m_darkTheme = false;
    class QPropertyAnimation *m_fadeAnim = nullptr;
    class QGraphicsOpacityEffect *m_opEffect = nullptr;
    class QSettings *m_settings = nullptr;
    class QPushButton *m_searchBtn = nullptr;
    bool issearched = false;
    bool m_targetDark = false;
};