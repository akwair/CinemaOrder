#pragma once
#include <QMainWindow>
class QSqlTableModel;
class TicketController;
class Database;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(Database &db, TicketController &controller, QWidget *parent = nullptr);
private slots:
    void onAdd();
    void onDelete();
    void onSell();
    void onRefund();
    void onImport();
    void onExport();
    void onSearch();
    void onSort();
private:
    TicketController &m_ctrl;
    QSqlTableModel *m_model;
    void refresh();
};
