#include "mainwindow.h"
#include "seatselectiondialog.h"
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
    addButton(tr("查找"), QIcon(":/icons/icons/search.svg"), SLOT(onSearch()));
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

    setWindowTitle("电影票管理");
    statusBar()->showMessage("就绪");
    setMinimumSize(800, 480);

    // prepare fade animation for theme transition
    m_opEffect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(m_opEffect);
    m_fadeAnim = new QPropertyAnimation(m_opEffect, "opacity", this);
    m_fadeAnim->setDuration(220);

    // settings for theme persistence
    m_settings = new QSettings("CinemaOrder", "CinemaApp", this);
    m_darkTheme = m_settings->value("ui/dark", false).toBool();
    // apply persisted theme immediately (no fade) to avoid transient mismatch
    if (m_darkTheme) {
        qApp->setStyleSheet(R"(
            QMainWindow { background: #2b2b2b; color: #ddd; }
            QTableView { background: #232323; color: #ddd; gridline-color: #444; selection-background-color: #3a7bd5; }
            QHeaderView::section { background: #2f2f2f; color: #ddd; }
            QDockWidget { background: #383838; }
            QDockWidget::title { background: #2f2f2f; color: #ddd; padding: 4px; }
            QPushButton { background: #3a3a3a; color: #fff; border: 1px solid #4a4a4a; padding: 6px 12px; border-radius: 4px; }
            QPushButton:hover { background: #4a4a4a; border: 1px solid #5a5a5a; }
            QPushButton:pressed { background: #2f2f2f; border: 1px solid #3a3a3a; }
        )");
    }
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
}

void MainWindow::onSort()
{
    QStringList items; items << "showDate" << "showTime" << "price";
    bool ok; QString choice = QInputDialog::getItem(this, "排序", "依据:", items, 0, false, &ok);
    if (!ok || choice.isEmpty()) return;
    m_model->setSort(m_model->fieldIndex(choice), Qt::AscendingOrder);
    refresh();
}

void MainWindow::onToggleTheme()
{
    // fade out, switch stylesheet, fade in
    bool targetDark = !m_darkTheme;
    m_fadeAnim->stop();
    m_fadeAnim->setStartValue(1.0); m_fadeAnim->setEndValue(0.0);
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this, targetDark]() {
        if (targetDark) {
            qApp->setStyleSheet(R"(
                QMainWindow { background: #2b2b2b; color: #ddd; }
                QTableView { background: #232323; color: #ddd; gridline-color: #444; selection-background-color: #3a7bd5; }
                QHeaderView::section { background: #2f2f2f; color: #ddd; }
                QDockWidget { background: #383838; }
                QDockWidget::title { background: #2f2f2f; color: #ddd; padding: 4px; }
                QPushButton { background: #3a3a3a; color: #fff; border: 1px solid #4a4a4a; padding: 6px 12px; border-radius: 4px; }
                QPushButton:hover { background: #4a4a4a; border: 1px solid #5a5a5a; }
                QPushButton:pressed { background: #2f2f2f; border: 1px solid #3a3a3a; }
            )");
        } else {
            qApp->setStyleSheet(R"(
                QMainWindow { background: #f7f9fc; color: #222; }
                QTableView { background: #fff; color: #222; gridline-color: #e6eef8; selection-background-color: #87cefa; }
                QHeaderView::section { background: #e9f2fb; color: #222; }
                QDockWidget { background: #eef6ff; }
                QDockWidget::title { background: #e0eef8; color: #222; padding: 4px; }
                QPushButton { background: #ffffff; color: #222; border: 1px solid #d0d8e8; padding: 6px 12px; border-radius: 4px; }
                QPushButton:hover { background: #eef6ff; border: 1px solid #a0c0e8; }
                QPushButton:pressed { background: #dce7f4; border: 1px solid #80a0d8; }
            )");
        }
        // set back to visible
        m_fadeAnim->setStartValue(0.0); m_fadeAnim->setEndValue(1.0);
        m_fadeAnim->disconnect();
        m_fadeAnim->start();
    });
    m_fadeAnim->start();
    m_darkTheme = targetDark;
    m_settings->setValue("ui/dark", m_darkTheme);
}
