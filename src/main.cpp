#include <QApplication>
#include "model/database.h"
#include "controller/ticketcontroller.h"
#include "view/mainwindow.h"
#include "view/usermainwindow.h"
#include "auth/authmanager.h"
#include "view/logindialog.h"
#include "view/logindialog.h"
#include "auth/authmanager.h"

// 主程序入口
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    // 应用Fusion样式
    QApplication::setStyle("Fusion");
    // 自定义样式表
    QString style = R"(
        QMainWindow { background: #f7f9fc; }
        QTableView { gridline-color: #e6eef8; selection-background-color: #cdd4d9ff; alternate-background-color: #eea317ff; }
        QHeaderView::section { background: #e9f2fb; padding: 4px; }
        QDockWidget { background: #eef6ff; }
        QDockWidget::title { background: #e0eef8; padding: 4px; }
        QPushButton { padding: 6px 12px; border-radius: 4px; background: #ffffff; color: #222; border: 1px solid #d0d8e8; outline: none; }
        QPushButton:hover { background: #eef6ff; border: 1px solid #bbcde2ff; }
        QPushButton:pressed { background: #dce7f4; border: 1px solid #80a0d8; }
        QPushButton:released { background: #ffffff; border: 1px solid #d0d8e8; }
        QPushButton:focus { outline: none; }
    )";
    // 应用样式表
    app.setStyleSheet(style);

    // 数据库路径：始终使用项目根目录（build 的上一级）下的 tickets.db
    QString appDir = QCoreApplication::applicationDirPath();
    QString dbPath = QDir(appDir).absoluteFilePath("../tickets.db");
    dbPath = QDir::cleanPath(dbPath); // 规范化路径

    // 初始化数据库连接
    Database db(dbPath);
    if (!db.open()) {
        qCritical() << "无法打开数据库";
        return 1;
    }
    // 创建票务业务控制器
    TicketController ctrl(db);
    // 用户认证
    // 创建认证管理器和登录对话框
    AuthManager auth(db);
    LoginDialog loginDlg(auth);
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }

    // 根据用户角色显示对应窗口
    int userRole = loginDlg.getRole();
    if (auth.isAdmin(userRole)) {
        // 管理员窗口（完整功能）
        MainWindow w(db, ctrl, loginDlg.getUsername());
        w.show();
        return app.exec();
    } else {
        // 普通用户窗口
        UserMainWindow w(db, ctrl, loginDlg.getUsername());
        w.show();
        return app.exec();
    }
}
