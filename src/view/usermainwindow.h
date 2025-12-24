#pragma once
#include <QMainWindow>
class QSqlTableModel;
class TicketController;
class Database;

// 普通用户主窗口
class UserMainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit UserMainWindow(Database &db, TicketController &controller, const QString &username, QWidget *parent = nullptr);
private slots:
    // 购票操作
    void onBuyTicket();
    // 退票操作
    void onRefund();
    // 查询票务
    void onSearch();
    // 排序票务
    void onSort();
    // 主题切换
    void onToggleTheme();
    // 应用主题
    void applyTheme(bool dark);
    // 淡入淡出完成
    void onFadeFinished();
    // 恢复显示
    void onRestore();
    // 查看我的购票
    void onViewMyTickets();
    // 查看电影详情
    void onViewMovieDetail();
private:
    TicketController &m_ctrl;   // 票务控制器
    QSqlTableModel *m_model;    // 数据模型
    Database &m_db;             // 数据库
    QString m_username;         // 用户名
    void refresh();
    // UI工具部件
    bool m_darkTheme = false;
    class QPropertyAnimation *m_fadeAnim = nullptr;
    class QGraphicsOpacityEffect *m_opEffect = nullptr;
    class QSettings *m_settings = nullptr;
    class QPushButton *m_searchBtn = nullptr;
    bool issearched = false;
    bool m_targetDark = false;
};