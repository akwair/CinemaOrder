#include "usermainwindow.h"
#include "seatselectiondialog.h"
#include "moviedetaildialog.h"
#include <QAction>
#include <QToolBar>
#include <QTableView>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStatusBar>
#include <QSizePolicy>
#include <QSqlQuery>
#include <QInputDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QStyle>
#include <QApplication>
#include <QFont>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QSettings>
#include "../controller/ticketcontroller.h"
#include <QSqlError>
#include <QLineEdit>
#include <QStringList>
#include <Qt>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTableWidget>
#include "../model/database.h"

UserMainWindow::UserMainWindow(Database &db, TicketController &controller, const QString &username, QWidget *parent)
    : QMainWindow(parent)
    , m_ctrl(controller)
    , m_model(new QSqlTableModel(this, db.db()))
    , m_db(db)
    , m_username(username)
{
    //数据模型设置
    m_model->setTable("tickets");
    m_model->select();

    //表格视图创建与设置
    auto *view = new QTableView(this);
    view->setModel(m_model);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setVisible(false);
    view->setAlternatingRowColors(true);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    view->verticalHeader()->setDefaultSectionSize(36);

    QFont f = view->font();
    f.setPointSize(10);
    view->setFont(f);
    setCentralWidget(view);

    // 连接双击信号到查看详情功能
    connect(view, &QTableView::doubleClicked, this, &UserMainWindow::onViewMovieDetail);

    // UI列隐藏
    int idCol = m_model->fieldIndex("id");
    if (idCol >= 0) view->hideColumn(idCol);

    // 左侧功能边栏创建
    auto *m_sideDock = new QDockWidget(tr("功能"), this);
    m_sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QWidget *sideWidget = new QWidget(this);
    QVBoxLayout *sideLayout = new QVBoxLayout(sideWidget);

    //统一创建边栏按钮
    auto addButton = [&](const QString &text, const QIcon &icon, const char *slot){
        QPushButton *b = new QPushButton(icon, text, this);
        b->setIconSize(QSize(20,20));
        b->setMinimumHeight(38);
        b->setCursor(Qt::PointingHandCursor);
        sideLayout->addWidget(b);
        connect(b, SIGNAL(clicked()), this, slot);
        return b;
    };

    // 用户功能按钮
    QStyle *st = QApplication::style();

    addButton(tr("买票"), QIcon(":/icons/icons/sell.svg"), SLOT(onBuyTicket()));
    addButton(tr("退票"), QIcon(":/icons/icons/refund.svg"), SLOT(onRefund()));
    sideLayout->addSpacing(8);

    addButton(tr("我的票务"), QIcon(":/icons/icons/ticket.svg"), SLOT(onViewMyTickets()));
    sideLayout->addSpacing(8);

    m_searchBtn = addButton(tr("查找"), QIcon(":/icons/icons/search.svg"), SLOT(onSearch()));
    addButton(tr("详情"), QIcon(":/icons/icons/info.svg"), SLOT(onViewMovieDetail()));
    addButton(tr("排序"), QIcon(":/icons/icons/sort.svg"), SLOT(onSort()));

    sideLayout->addStretch();

    // 主题切换按钮
    QPushButton *themeBtn = new QPushButton(QIcon(":/icons/icons/theme.svg"), tr("切换主题"), this);
    themeBtn->setMinimumHeight(36);
    connect(themeBtn, &QPushButton::clicked, this, &UserMainWindow::onToggleTheme);
    sideLayout->addWidget(themeBtn);

    // 边栏完成设置
    sideWidget->setLayout(sideLayout);
    m_sideDock->setWidget(sideWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_sideDock);

    // 设置停靠窗口特性
    m_sideDock->setFeatures(QDockWidget::DockWidgetClosable |
                           QDockWidget::DockWidgetMovable |
                           QDockWidget::DockWidgetFloatable);

    // 工具栏设置
    auto *m_toggleDockAction = new QAction(tr("功能"), this);
    m_toggleDockAction->setCheckable(true);
    m_toggleDockAction->setChecked(m_sideDock->isVisible());
    connect(m_toggleDockAction, &QAction::toggled, m_sideDock, &QWidget::setVisible);
    connect(m_sideDock, &QDockWidget::visibilityChanged, m_toggleDockAction, &QAction::setChecked);

    QToolBar *tb = addToolBar(tr("主工具"));
    tb->setMovable(false);
    tb->addAction(m_toggleDockAction);

    // 主窗口基本设置
    setWindowTitle("电影票购买");
    statusBar()->showMessage("双击电影行查看详情 | 就绪");
    setMinimumSize(1100, 600);

    // 主题切换动画效果
    m_opEffect = new QGraphicsOpacityEffect(this);
    m_opEffect->setOpacity(1.0);
    this->setGraphicsEffect(m_opEffect);

    m_fadeAnim = new QPropertyAnimation(m_opEffect, "opacity", this);
    m_fadeAnim->setDuration(220);
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, &UserMainWindow::onFadeFinished);

    // 主题持久化设置
    m_settings = new QSettings("CinemaOrder", "CinemaApp", this);
    m_darkTheme = m_settings->value("ui/dark", false).toBool();
    applyTheme(m_darkTheme);
}

void UserMainWindow::refresh()
{
    m_model->select();
}

void UserMainWindow::onBuyTicket()
{
    auto *view = qobject_cast<QTableView*>(centralWidget());
    if (!view) return;

    if (!view->selectionModel()->hasSelection()) {
        QMessageBox::information(this, "提示", "请选择场次");
        return;
    }

    int row = view->selectionModel()->selectedRows().first().row();
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();
    int capacity = m_model->data(m_model->index(row, m_model->fieldIndex("capacity"))).toInt();
    int sold = m_model->data(m_model->index(row, m_model->fieldIndex("sold"))).toInt();

    if (sold >= capacity) {
        QMessageBox::information(this, "提示", "该场次已售罄");
        return;
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    if(currentDateTime.date() > m_model->data(m_model->index(row, m_model->fieldIndex("showDate"))).toDate() ||
       (currentDateTime.date() == m_model->data(m_model->index(row, m_model->fieldIndex("showDate"))).toDate() &&
        currentDateTime.time() > m_model->data(m_model->index(row, m_model->fieldIndex("showTime"))).toTime())){
        QMessageBox::information(this, "提示", "该场次已结束，无法购票");
        return;
    }

    SeatSelectionDialog dlg(m_db, id, capacity, 0, m_username, this);
    if (dlg.exec() != QDialog::Accepted) return;

    auto seats = dlg.selectedSeats();
    if (seats.isEmpty()) return;

    int toSell = seats.size();
    if (sold + toSell > capacity) {
        QMessageBox::warning(this, "购票失败", "选择的座位超过剩余票数");
        return;
    }

    QSqlDatabase db = m_model->database();
    db.transaction();

    QSqlQuery q(db);
    q.prepare("UPDATE seats SET status = 1, username = ?, user_fullname = ?, user_phonenumber = ?, user_email = ? WHERE id = ?");
    for (const SeatInfo &s : seats) {
        q.addBindValue(m_username);
        q.addBindValue(s.userFullName);
        q.addBindValue(s.userPhoneNumber);
        q.addBindValue(s.userEmail);
        q.addBindValue(s.id);
        if (!q.exec())
            qWarning() << "更新座位状态失败:" << q.lastError().text();
        q.finish();
    }

    QSqlQuery q2(db);
    q2.prepare("UPDATE tickets SET sold = sold+?,remain=remain-? WHERE id = ? AND sold+?<=capacity");
    q2.addBindValue(toSell);
    q2.addBindValue(toSell);
    q2.addBindValue(id);
    q2.addBindValue(toSell);

    bool ok = q2.exec();
    if (!ok) {
        db.rollback();
        QMessageBox::warning(this, "购票失败", "更新票务信息失败");
    } else {
        db.commit();
        QMessageBox::information(this, "成功", "购票成功！");
    }

    refresh();
}

void UserMainWindow::onRefund()
{
    auto *view = qobject_cast<QTableView*>(centralWidget());
    if (!view) return;

    if (!view->selectionModel()->hasSelection()) {
        QMessageBox::information(this, "提示", "请选择场次");
        return;
    }

    int row = view->selectionModel()->selectedRows().first().row();
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();
    int capacity = m_model->data(m_model->index(row, m_model->fieldIndex("capacity"))).toInt();
    int sold = m_model->data(m_model->index(row, m_model->fieldIndex("sold"))).toInt();

    if (sold == 0) {
        QMessageBox::information(this, "提示", "该场次无可退票");
        return;
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    if(currentDateTime.date() > m_model->data(m_model->index(row, m_model->fieldIndex("showDate"))).toDate() ||
       (currentDateTime.date() == m_model->data(m_model->index(row, m_model->fieldIndex("showDate"))).toDate() &&
        currentDateTime.time() > m_model->data(m_model->index(row, m_model->fieldIndex("showTime"))).toTime())){
        QMessageBox::information(this, "提示", "该场次已开始，无法退票");
        return;
    }

    SeatSelectionDialog dlg(m_db, id, capacity, 1, m_username, this);
    if (dlg.exec() != QDialog::Accepted) return;

    QSqlDatabase db = m_model->database();
    auto seats = dlg.selectedSeats();
    if(seats.isEmpty()) return;

    int toRefund = seats.size();
    if (sold - toRefund < 0) {
        QMessageBox::warning(this, "退票失败", "选择的座位超过已售票数");
        return;
    }

    db.transaction();
    QSqlQuery q1(db);
    q1.prepare("UPDATE seats SET status=0 WHERE id = ? AND status=1");

    for(const auto &s : seats){
        q1.addBindValue(s.id);
        if(!q1.exec()){
            qWarning() << "退票失败:" << q1.lastError().text();
        }
        q1.finish();
    }

    QSqlQuery q2(db);
    q2.prepare("UPDATE tickets SET sold=sold-?, remain=remain+? WHERE id=? AND sold-?>=0");
    q2.addBindValue(toRefund);
    q2.addBindValue(toRefund);
    q2.addBindValue(id);
    q2.addBindValue(toRefund);

    bool ok = q2.exec();
    if(!ok){
        db.rollback();
        QMessageBox::warning(this, "退票失败", "更新票务信息失败");
    }else{
        db.commit();
        QMessageBox::information(this, "成功", "退票成功！");
    }
    refresh();
}

void UserMainWindow::onSearch()
{
    QString key = QInputDialog::getText(this, "查找", "电影或影院关键字:");
    if (key.isEmpty()) return;
    QString filter = QString("movieName LIKE '%%1%%' OR cinemaName LIKE '%%1%%'").arg(key);
    m_model->setFilter(filter);
    refresh();
    if (m_searchBtn) {
        QObject::disconnect(m_searchBtn, SIGNAL(clicked()), this, SLOT(onSearch()));
        connect(m_searchBtn, &QPushButton::clicked, this, &UserMainWindow::onRestore);
        m_searchBtn->setText(tr("还原"));
        issearched = true;
    }
}

void UserMainWindow::onRestore()
{
    m_model->setFilter("");
    refresh();
    if (m_searchBtn) {
        QObject::disconnect(m_searchBtn, SIGNAL(clicked()), this, SLOT(onRestore()));
        connect(m_searchBtn, &QPushButton::clicked, this, &UserMainWindow::onSearch);
        m_searchBtn->setText(tr("查找"));
        issearched = false;
    }
}

void UserMainWindow::onViewMyTickets()
{
    QDialog dlg(this);
    dlg.setWindowTitle("我的票务");
    dlg.resize(800, 600);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QTableWidget *table = new QTableWidget(&dlg);
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"电影名称", "影院名称", "座位", "放映日期", "放映时间", "状态"});

    QSqlQuery q(m_db.db());
    q.prepare("SELECT t.movieName, t.cinemaName, s.label, t.showDate, t.showTime, s.status "
              "FROM seats s JOIN tickets t ON s.ticket_id = t.id "
              "WHERE s.username = ? AND s.status = 1");
    q.addBindValue(m_username);
    if (q.exec()) {
        int row = 0;
        while (q.next()) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
            table->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
            table->setItem(row, 2, new QTableWidgetItem(q.value(2).toString()));
            table->setItem(row, 3, new QTableWidgetItem(q.value(3).toString()));
            table->setItem(row, 4, new QTableWidgetItem(q.value(4).toString()));
            table->setItem(row, 5, new QTableWidgetItem("已购买"));
            row++;
        }
    }

    table->resizeColumnsToContents();
    layout->addWidget(table);

    QPushButton *closeBtn = new QPushButton("关闭", &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg.exec();
}

void UserMainWindow::onSort()
{
    QStringList items; items << "showDate" << "showTime" << "price";
    bool ok; QString choice = QInputDialog::getItem(this, "排序", "依据:", items, 0, false, &ok);
    if (!ok || choice.isEmpty()) return;
    m_model->setSort(m_model->fieldIndex(choice), Qt::AscendingOrder);
    refresh();
}

void UserMainWindow::onToggleTheme()
{
    m_targetDark = !m_darkTheme;
    m_fadeAnim->setStartValue(1.0);
    m_fadeAnim->setEndValue(0.0);
    m_fadeAnim->start();
}

void UserMainWindow::applyTheme(bool dark)
{
    auto loadSheet = [](const QString &filename)->QString{
        QStringList candidates;
        QString appDir = QCoreApplication::applicationDirPath();
        candidates << (appDir + "/styles/" + filename);
        candidates << (appDir + "/../styles/" + filename);
        candidates << (QDir::currentPath() + "/styles/" + filename);
        for (const QString &p : candidates) {
            QFile f(p);
            if (f.exists() && f.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&f);
                return in.readAll();
            }
        }
        return QString();
    };

    QString sheet;
    if (dark) sheet = loadSheet("dark.qss");
    else sheet = loadSheet("light.qss");
    if (!sheet.isEmpty()) {
        qApp->setStyleSheet(sheet);
        return;
    }

    // fallback inline styles
    if (dark) {
        QString darkStyle = R"(
            QMainWindow { background: #2b2b2b; color: #ffffff; }
            QTableView { gridline-color: #555555; selection-background-color: #555555; alternate-background-color: #3a3a3a; background: #2b2b2b; color: #ffffff; }
            QHeaderView::section { background: #3a3a3a; color: #ffffff; padding: 4px; }
            QDockWidget { background: #3a3a3a; color: #ffffff; }
            QDockWidget::title { background: #4a4a4a; color: #ffffff; padding: 4px; }
            QPushButton { padding: 6px 12px; border-radius: 4px; background: #555555; color: #ffffff; border: 1px solid #777777; outline: none; }
            QPushButton:hover { background: #666666; border: 1px solid #999999; }
            QPushButton:pressed { background: #444444; border: 1px solid #555555; }
            QPushButton:focus { outline: none; }
        )";
        qApp->setStyleSheet(darkStyle);
    } else {
        QString lightStyle = R"(
            QMainWindow { background: #f7f9fc; }
            QTableView { gridline-color: #e6eef8; selection-background-color: #cdd4d9ff; alternate-background-color: #eea317ff; }
            QHeaderView::section { background: #e9f2fb; padding: 4px; }
            QDockWidget { background: #eef6ff; }
            QDockWidget::title { background: #e0eef8; padding: 4px; }
            QPushButton { padding: 6px 12px; border-radius: 4px; background: #ffffff; color: #222; border: 1px solid #d0d8e8; outline: none; }
            QPushButton:hover { background: #eef6ff; border: 1px solid #bbcde2ff; }
            QPushButton:pressed { background: #dce7f4; border: 1px solid #80a0d8; }
            QPushButton:focus { outline: none; }
        )";
        qApp->setStyleSheet(lightStyle);
    }
    m_darkTheme = dark;
    if (m_settings) m_settings->setValue("ui/dark", dark);
}

void UserMainWindow::onFadeFinished()
{
    applyTheme(m_targetDark);
    m_fadeAnim->setStartValue(0.0);
    m_fadeAnim->setEndValue(1.0);
    m_fadeAnim->start();
}

void UserMainWindow::onViewMovieDetail()
{
    // 检查用户是否选择了电影
    auto *view = qobject_cast<QTableView*>(centralWidget());
    if (!view) return;
    
    auto selection = view->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一个电影场次");
        return;
    }
    
    int row = selection.first().row();
    int ticketId = m_model->data(m_model->index(row, 0)).toInt(); // 获取ID列的值
    
    // 从数据库查询电影详情
    QSqlQuery q(m_db.db());
    q.prepare("SELECT movieName, description, director, actors, genre, rating, poster FROM tickets WHERE id = ?");
    q.addBindValue(ticketId);
    
    if (q.exec() && q.next()) {
        QString movieName = q.value(0).toString();
        QString description = q.value(1).toString();
        QString director = q.value(2).toString();
        QString actors = q.value(3).toString();
        QString genre = q.value(4).toString();
        double rating = q.value(5).toDouble();
        QString poster = q.value(6).toString();
        
        // 显示电影详情对话框
        MovieDetailDialog dlg(this);
        dlg.setMovieInfo(movieName, description, director, actors, genre, rating, poster);
        dlg.setEditMode(false); // 用户只能查看，不能编辑
        dlg.exec();
    } else {
        QMessageBox::warning(this, "错误", "无法获取电影详情信息");
    }
}