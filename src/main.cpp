#include <QApplication>
#include <QDebug>
#ifdef _WIN32
#include <windows.h>
#include <locale.h>
#endif
#include "model/database.h"
#include "controller/ticketcontroller.h"
#include "view/mainwindow.h"
#include "auth/authmanager.h"
#include "view/logindialog.h"

int main(int argc, char **argv)
{
#ifdef _WIN32
    // set C locale and console codepage to UTF-8 for proper console output
    setlocale(LC_ALL, "");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    QApplication app(argc, argv);
    // Use Fusion style and a light modern stylesheet
    QApplication::setStyle("Fusion");
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
    app.setStyleSheet(style);
    Database db("tickets.db");
    if (!db.open()) {
        qCritical() << "无法打开数据库";
        return 1;
    }
    TicketController ctrl(db);
    // Authentication
    #include "view/logindialog.h"
    #include "auth/authmanager.h"
    AuthManager auth(db);
    LoginDialog loginDlg(auth);
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0; // user cancelled or failed to login
    }
    MainWindow w(db, ctrl);
    w.show();
    return app.exec();
}
