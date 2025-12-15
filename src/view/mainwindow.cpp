#include "mainwindow.h"
#include "seatselectiondialog.h"
#include <QAction>
#include <QToolBar>
#include <QAction>
#include <QTableView>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QStatusBar>
#include <QSizePolicy>
#include <QSqlQuery>
#include <QInputDialog>
#include <QFileDialog>
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
#include "ticketformdialog.h"
#include <QSqlError>
#include <QLineEdit>
#include <QStringList>
#include <Qt>
#include "../model/database.h"

MainWindow::MainWindow(Database &db, TicketController &controller, QWidget *parent)
    : QMainWindow(parent)
    , m_ctrl(controller)
    , m_model(new QSqlTableModel(this, db.db()))
    , m_db(db)
{
    m_model->setTable("tickets");
    m_model->select();

    auto *view = new QTableView(this);
    view->setModel(m_model);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    view->verticalHeader()->setVisible(false);
    view->setAlternatingRowColors(true);
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    view->verticalHeader()->setDefaultSectionSize(36); // row height
    QFont f = view->font(); f.setPointSize(10); view->setFont(f);
    setCentralWidget(view);

    // hide internal id column from UI
    int idCol = m_model->fieldIndex("id");
    if (idCol >= 0) view->hideColumn(idCol);

    // Create a left sidebar (dock) with main actions and theme toggle
    m_sideDock = new QDockWidget(tr("功能"), this);
    m_sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QWidget *sideWidget = new QWidget(this);
    QVBoxLayout *sideLayout = new QVBoxLayout(sideWidget);

    auto addButton = [&](const QString &text, const QIcon &icon, const char *slot){
        QPushButton *b = new QPushButton(icon, text, this);
        b->setIconSize(QSize(20,20));
        b->setMinimumHeight(38);
        b->setCursor(Qt::PointingHandCursor);
        sideLayout->addWidget(b);
        connect(b, SIGNAL(clicked()), this, slot);
        return b;
    };

    // use style standard icons for portability
    QStyle *st = QApplication::style();
    addButton(tr("新增"), QIcon(":/icons/icons/add.svg"), SLOT(onAdd()));
    addButton(tr("删除"), QIcon(":/icons/icons/delete.svg"), SLOT(onDelete()));
    addButton(tr("售票"), QIcon(":/icons/icons/sell.svg"), SLOT(onSell()));
    addButton(tr("退票"), QIcon(":/icons/icons/refund.svg"), SLOT(onRefund()));
    sideLayout->addSpacing(8);
    addButton(tr("导入"), QIcon(":/icons/icons/import.svg"), SLOT(onImport()));
    addButton(tr("导出"), QIcon(":/icons/icons/export.svg"), SLOT(onExport()));
    sideLayout->addSpacing(8);
    m_searchBtn = addButton(tr("查找"), QIcon(":/icons/icons/search.svg"), SLOT(onSearch()));
    addButton(tr("排序"), QIcon(":/icons/icons/sort.svg"), SLOT(onSort()));

    sideLayout->addStretch();
    // theme toggle
    QPushButton *themeBtn = new QPushButton(QIcon(":/icons/icons/theme.svg"), tr("切换主题"), this);
    themeBtn->setMinimumHeight(36);
    connect(themeBtn, &QPushButton::clicked, this, &MainWindow::onToggleTheme);
    sideLayout->addWidget(themeBtn);

    sideWidget->setLayout(sideLayout);
    m_sideDock->setWidget(sideWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_sideDock);

    // ensure dock can be reopened via toolbar action
    m_sideDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_toggleDockAction = new QAction(tr("功能"), this);
    m_toggleDockAction->setCheckable(true);
    m_toggleDockAction->setChecked(m_sideDock->isVisible());
    connect(m_toggleDockAction, &QAction::toggled, m_sideDock, &QWidget::setVisible);
    connect(m_sideDock, &QDockWidget::visibilityChanged, m_toggleDockAction, &QAction::setChecked);
    QToolBar *tb = addToolBar(tr("主工具"));
    tb->setMovable(false);
    tb->addAction(m_toggleDockAction);

    setWindowTitle("电影票管理");
    statusBar()->showMessage("就绪");
    setMinimumSize(800, 480);

    // prepare fade animation for theme transition
    m_opEffect = new QGraphicsOpacityEffect(this);
    m_opEffect->setOpacity(1.0);
    this->setGraphicsEffect(m_opEffect);
    m_fadeAnim = new QPropertyAnimation(m_opEffect, "opacity", this);
    m_fadeAnim->setDuration(220);
    // connect finished once to handler
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, &MainWindow::onFadeFinished);

    // settings for theme persistence
    m_settings = new QSettings("CinemaOrder", "CinemaApp", this);
    m_darkTheme = m_settings->value("ui/dark", false).toBool();
    // apply persisted theme immediately (no fade)
    applyTheme(m_darkTheme);
}

void MainWindow::refresh()
{
    m_model->select();
}

void MainWindow::onAdd()
{
    // Use TicketFormDialog for a nicer add form
    TicketFormDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;
    Ticket t = dlg.ticket();
    QSqlQuery q(m_model->database());
    q.prepare("INSERT INTO tickets (movieName,cinemaName,showDate,showTime,duration,price,hall,capacity,sold) VALUES (?,?,?,?,?,?,?,?,?)");
    q.addBindValue(t.movieName);
    q.addBindValue(t.cinemaName);
    q.addBindValue(t.showDate);
    q.addBindValue(t.showTime);
    q.addBindValue(t.duration);
    q.addBindValue(t.price);
    q.addBindValue(t.hall);
    q.addBindValue(t.capacity);
    q.addBindValue(0);
    if (!q.exec()) QMessageBox::warning(this, "错误", q.lastError().text());
    refresh();
    // clear selection so newly-added row is not left selected
    auto *view = qobject_cast<QTableView*>(centralWidget());
    if (view && view->selectionModel()) {
        qDebug() << "当前选中行数:" << view->selectionModel()->selectedRows().size();
        qDebug() << "清除前选中行:" << view->selectionModel()->selectedRows();
        
        view->selectionModel()->clearSelection();
        
        qDebug() << "清除后选中行数:" << view->selectionModel()->selectedRows().size();
    }
}

void MainWindow::onDelete()
{
    auto *view = qobject_cast<QTableView*>(centralWidget()); if (!view) return;
    auto sel = view->selectionModel();
    if (!sel->hasSelection()) { QMessageBox::information(this, "提示", "请先选择一行"); return; }
    int row = sel->selectedRows().first().row();
    m_model->removeRow(row);
    m_model->submitAll();
    refresh();
}

void MainWindow::onSell()
{
    auto *view = qobject_cast<QTableView*>(centralWidget()); if (!view) return;
    if (!view->selectionModel()->hasSelection()) { QMessageBox::information(this, "提示", "请选择场次"); return; }
    int row = view->selectionModel()->selectedRows().first().row();
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();
    int capacity = m_model->data(m_model->index(row, m_model->fieldIndex("capacity"))).toInt();
    int sold = m_model->data(m_model->index(row, m_model->fieldIndex("sold"))).toInt();
    if (sold >= capacity) { QMessageBox::information(this, "提示", "该场次已售罄"); return; }

    // show seat selection dialog
    SeatSelectionDialog dlg(m_db, id, capacity, this);
    if (dlg.exec() != QDialog::Accepted) return;
    auto seats = dlg.selectedSeats();
    if (seats.isEmpty()) return; // nothing selected

    int toSell = seats.size();
    if (sold + toSell > capacity) { QMessageBox::warning(this, "售票失败", "选择的座位超过剩余票数"); return; }

    // mark seats as sold and update tickets.sold in a transaction
    QSqlDatabase db = m_model->database();
    db.transaction();
    QSqlQuery q(db);
    q.prepare("UPDATE seats SET status = 1 WHERE id = ?");
    for (const SeatInfo &s : seats) {
        q.addBindValue(s.id);
        if (!q.exec()) qWarning() << "Failed to mark seat sold:" << q.lastError().text();
        q.finish();
    }
    QSqlQuery q2(db);
    q2.prepare("UPDATE tickets SET sold = sold + ? WHERE id = ?");
    q2.addBindValue(toSell); q2.addBindValue(id);
    bool ok = q2.exec();
    if (!ok) {
        db.rollback();
        QMessageBox::warning(this, "售票失败", "更新票务信息失败");
    } else {
        db.commit();
    }
    refresh();
}

void MainWindow::onRefund()
{
    bool ok; int qty = QInputDialog::getInt(this, "退票", "数量:", 1, 1, 10000, 1, &ok); if (!ok) return;
    auto *view = qobject_cast<QTableView*>(centralWidget()); if (!view) return;
    if (!view->selectionModel()->hasSelection()) { QMessageBox::information(this, "提示", "请选择场次"); return; }
    int row = view->selectionModel()->selectedRows().first().row();
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();
    QSqlQuery q(m_model->database());
    q.prepare("UPDATE tickets SET sold = sold - ? WHERE id = ? AND sold - ? >= 0");
    q.addBindValue(qty); q.addBindValue(id); q.addBindValue(qty);
    if (!q.exec() || q.numRowsAffected() == 0) QMessageBox::warning(this, "退票失败", "可能退票数非法或发生错误");
    refresh();
}

void MainWindow::onImport()
{
    QString path = QFileDialog::getOpenFileName(this, "导入文件", QString(), "CSV Files (*.csv);;All Files (*)");
    if (path.isEmpty()) return;
    m_ctrl.loadFromFile(path, true);
    refresh();
}

void MainWindow::onExport()
{
    QString path = QFileDialog::getSaveFileName(this, "导出文件", QString(), "CSV Files (*.csv);;All Files (*)");
    if (path.isEmpty()) return;
    m_ctrl.saveToFile(path);
}

void MainWindow::onSearch()
{
    QString key = QInputDialog::getText(this, "查找", "电影或影院关键字:");
    if (key.isEmpty()) return;
    QString filter = QString("movieName LIKE '%%1%%' OR cinemaName LIKE '%%1%%'").arg(key);
    m_model->setFilter(filter);
    refresh();
    // change search button into a restore button
    if (m_searchBtn) {
        // disconnect previous connection and connect to onRestore
        QObject::disconnect(m_searchBtn, SIGNAL(clicked()), this, SLOT(onSearch()));
        connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::onRestore);
        m_searchBtn->setText(tr("还原"));
        issearched = true;
    }
}

void MainWindow::onRestore()
{
    // clear filter and restore original list
    m_model->setFilter("");
    refresh();
    if (m_searchBtn) {
        QObject::disconnect(m_searchBtn, SIGNAL(clicked()), this, SLOT(onRestore()));
        connect(m_searchBtn, &QPushButton::clicked, this, &MainWindow::onSearch);
        m_searchBtn->setText(tr("查找"));
        issearched = false;
    }
}

void MainWindow::onSort()
{
    QStringList items; items << "showDate" << "showTime" << "price";
    bool ok; QString choice = QInputDialog::getItem(this, "排序", "依据:", items, 0, false, &ok);
    if (!ok || choice.isEmpty()) return;
    m_model->setSort(m_model->fieldIndex(choice), Qt::AscendingOrder);
    refresh();
}

void MainWindow::applyTheme(bool dark)
{
    // try load external stylesheet files first (project styles folder)
    auto loadSheet = [](const QString &filename)->QString{
        QStringList candidates;
        QString appDir = QCoreApplication::applicationDirPath();
        candidates << (appDir + "/styles/" + filename);
        candidates << (appDir + "/../styles/" + filename); // when running from build/
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

    // fallback to built-in styles if external files not found
    if (dark) {
    qApp->setStyleSheet(R"(
        /* 暗色主题 */
        QMainWindow { 
            background: #1e1e1e; 
            color: #e0e0e0; 
        }
        QTableView { 
            background: #2d2d2d; 
            color: #e0e0e0; 
            gridline-color: #404040; 
            selection-background-color: #2c7be5;  /* 亮蓝色 */
            selection-color: white;  /* 选中文字白色 */
            alternate-background-color: #252525;
            border: 1px solid #404040;
            border-radius: 10px;
        }
        /* 选中行样式 */
        QTableView::item:selected {
            background: #2c7be5;  /* 明确指定选中背景 */
            color: white;         /* 明确指定选中文字颜色 */
            border-radius: 10px;
        }
        /* 交替行的选中状态 */
        QTableView::item:alternate:selected {
            background: #2c7be5;
            color: white;
            border-radius: 10px;
        }
        /* 焦点行的边框 */
        QTableView::item:focus {
            outline: none;
            border: none;
        }
        QHeaderView::section { 
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #333, stop:1 #2a2a2a);
            color: #e0e0e0; 
            padding: 8px 6px;
            border: 1px solid #404040;
            font-weight: 500;
        }
        QDockWidget { 
            background: #2a2a2a; 
            border: 1px solid #404040;
        }
        QDockWidget::title { 
            background: #333; 
            color: #e0e0e0; 
            padding: 6px 8px; 
            border-bottom: 1px solid #404040;
        }
        QPushButton { 
            background: #3a3a3a; 
            color: #fff; 
            border: 1px solid #4a4a4a; 
            padding: 8px 16px; 
            border-radius: 4px;
            min-width: 80px;
        }
        QPushButton:hover { 
            background: #4a4a4a; 
            border: 1px solid #5a5a5a; 
        }
        QPushButton:pressed { 
            background: #2a2a2a; 
            border: 1px solid #3a3a3a; 
        }
        QPushButton:disabled {
            background: #2d2d2d;
            color: #666;
            border: 1px solid #3a3a3a;
        }
    )");
} else {
    qApp->setStyleSheet(R"(
        /* 浅色主题 */
        QMainWindow { 
            background: #f7f9fc; 
            color: #2c3e50; 
        }
        QTableView { 
            background: #ffffff; 
            color: #333333;  /* 深灰色文字，比纯黑柔和 */
            gridline-color: #e6eef8; 
            selection-background-color: #4a90e2;  /* 明显的蓝色 */
            selection-color: white;  /* 选中文字白色 - 关键设置 */
            alternate-background-color: #f9fbfd;
            border: 1px solid #d0d8e8;
            border-radius: 6px;
        }
        /* 选中行样式 */
        QTableView::item:selected {
            background: #4a90e2;  /* 明确指定选中背景 */
            color: white;         /* 明确指定选中文字颜色 */
        }
        /* 交替行的选中状态 */
        QTableView::item:alternate:selected {
            background: #4a90e2;
            color: white;
        }
        /* 悬停效果 */
        QTableView::item:hover {
            background: #f0f7ff;
        }
        /* 选中行的悬停效果 */
        QTableView::item:selected:hover {
            background: #5aa0f2;  /* 悬停时稍微亮一点 */
            color: white;
        }
        /* 焦点行的边框 */
        QTableView::item:focus {
            outline: none;
            border: none;
        }
        QHeaderView::section { 
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                        stop:0 #e9f2fb, stop:1 #d8e5f5);
            color: #2c3e50; 
            padding: 10px 8px;
            border: 1px solid #d0d8e8;
            font-weight: 600;
        }
        QDockWidget { 
            background: #f0f7ff; 
            border: 1px solid #c8d4e8;
            border-radius: 6px;
        }
        QDockWidget::title { 
            background: #e0eef8; 
            color: #2c3e50; 
            padding: 8px; 
            border-bottom: 2px solid #c8d4e8;
            border-radius: 6px 6px 0 0;
        }
        QPushButton { 
            background: #ffffff; 
            color: #2c3e50; 
            border: 1px solid #d0d8e8; 
            padding: 8px 16px; 
            border-radius: 6px;
            min-width: 80px;
        }
        QPushButton:hover { 
            background: #eef6ff; 
            border: 1px solid #4a90e2; 
        }
        QPushButton:pressed { 
            background: #dce7f4; 
            border: 1px solid #2c7be5; 
        }
        QPushButton:disabled {
            background: #f5f5f5;
            color: #a0a0a0;
            border: 1px solid #e0e0e0;
        }
    )");
}
}

void MainWindow::onToggleTheme()
{
    // initiate fade-out; theme change will be applied in onFadeFinished
    m_targetDark = !m_darkTheme;
    m_fadeAnim->stop();
    m_fadeAnim->setStartValue(1.0);
    m_fadeAnim->setEndValue(0.0);
    m_fadeAnim->start();
}

void MainWindow::onFadeFinished()
{
    // if we've just faded out (opacity == 0), apply theme and fade back in
    if (m_opEffect->opacity() < 0.01) {
        applyTheme(m_targetDark);
        m_darkTheme = m_targetDark;
        m_settings->setValue("ui/dark", m_darkTheme);
        m_fadeAnim->setStartValue(0.0);
        m_fadeAnim->setEndValue(1.0);
        m_fadeAnim->start();
    }
}
