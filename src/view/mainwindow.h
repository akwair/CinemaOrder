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
    void onToggleTheme();
    void applyTheme(bool dark);
    void onFadeFinished();
private:
    TicketController &m_ctrl;
    QSqlTableModel *m_model;
    Database &m_db;
    void refresh();
    // UI helpers
    class QDockWidget *m_sideDock = nullptr;
    bool m_darkTheme = false;
    class QPropertyAnimation *m_fadeAnim = nullptr;
    class QGraphicsOpacityEffect *m_opEffect = nullptr;
    class QSettings *m_settings = nullptr;
    bool m_targetDark = false;
};
