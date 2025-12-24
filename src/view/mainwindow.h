#pragma once
#include <QMainWindow>
class QSqlTableModel;
class TicketController;
class Database;

// 管理员主窗口
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(Database &db, TicketController &controller, const QString &username = "admin", QWidget *parent = nullptr);
private slots:
    // 新增票务
    void onAdd();
    // 删除票务
    void onDelete();
    // 售票操作
    void onSell();
    // 退票操作
    void onRefund();
    // 导入数据
    void onImport();
    // 导出数据
    void onExport();
    // 查询票务
    void onSearch();
    // 排序票务
    void onSort();
    // 编辑电影详情
    void onEditMovieDetail();
    // 查看/编辑电影详情
    void onViewOrEditMovieDetail();
    // 主题切换
    void onToggleTheme();
    // 应用主题
    void applyTheme(bool dark);
    // 淡入淡出完成
    void onFadeFinished();
    // 恢复显示
    void onRestore();
    // 查看所有票务
    void onViewAllTickets();
private:
    TicketController &m_ctrl;   // 票务控制器
    QSqlTableModel *m_model;    // 数据模型
    Database &m_db;             // 数据库
    QString m_username;         // 管理员用户名
    void refresh();
    // UI工具部件
    class QDockWidget *m_sideDock = nullptr;
    bool m_darkTheme = false;
    class QPropertyAnimation *m_fadeAnim = nullptr;
    class QGraphicsOpacityEffect *m_opEffect = nullptr;
    class QSettings *m_settings = nullptr;
    class QAction *m_toggleDockAction = nullptr;
    bool m_targetDark = false;
    class QPushButton *m_searchBtn = nullptr;
    bool issearched=false;
};
