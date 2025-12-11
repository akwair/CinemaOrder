#include "mainwindow.h"
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
#include "../controller/ticketcontroller.h"
#include <QSqlError>
#include <QLineEdit>
#include <QStringList>
#include <Qt>
#include "../model/database.h"

MainWindow::MainWindow(Database &db, TicketController &controller, QWidget *parent)
    : QMainWindow(parent)
    , m_ctrl(controller)
    , m_model(new QSqlTableModel(this, db.db()))
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
    setCentralWidget(view);

    auto *tb = addToolBar("main");
    auto *actAdd = tb->addAction("新增"); connect(actAdd, &QAction::triggered, this, &MainWindow::onAdd);
    auto *actDel = tb->addAction("删除"); connect(actDel, &QAction::triggered, this, &MainWindow::onDelete);
    auto *actSell = tb->addAction("售票"); connect(actSell, &QAction::triggered, this, &MainWindow::onSell);
    auto *actRefund = tb->addAction("退票"); connect(actRefund, &QAction::triggered, this, &MainWindow::onRefund);
    tb->addSeparator();
    auto *actImport = tb->addAction("导入"); connect(actImport, &QAction::triggered, this, &MainWindow::onImport);
    auto *actExport = tb->addAction("导出"); connect(actExport, &QAction::triggered, this, &MainWindow::onExport);
    tb->addSeparator();
    auto *actSearch = tb->addAction("查找"); connect(actSearch, &QAction::triggered, this, &MainWindow::onSearch);
    auto *actSort = tb->addAction("排序"); connect(actSort, &QAction::triggered, this, &MainWindow::onSort);
    tb->addSeparator();
    auto *actExit = tb->addAction("退出"); connect(actExit, &QAction::triggered, this, &QMainWindow::close);

    setWindowTitle("电影票管理");
    statusBar()->showMessage("就绪");
    setMinimumSize(800, 480);
}

void MainWindow::refresh()
{
    m_model->select();
}

void MainWindow::onAdd()
{
    bool ok;
    QString movie = QInputDialog::getText(this, "新增", "电影名称:", QLineEdit::Normal, QString(), &ok);
    if (!ok || movie.isEmpty()) return;
    QString cinema = QInputDialog::getText(this, "新增", "影院名称:", QLineEdit::Normal, QString(), &ok); if (!ok) return;
    QString date = QInputDialog::getText(this, "新增", "放映日期 (YYYY-MM-DD):", QLineEdit::Normal, QString(), &ok); if (!ok) return;
    QString time = QInputDialog::getText(this, "新增", "放映时间 (HH:MM):", QLineEdit::Normal, QString(), &ok); if (!ok) return;
    int duration = QInputDialog::getInt(this, "新增", "电影时长(分钟):", 120, 1, 10000, 1, &ok); if (!ok) return;
    double price = QInputDialog::getDouble(this, "新增", "票价:", 60, 0, 100000, 2, &ok); if (!ok) return;
    QString hall = QInputDialog::getText(this, "新增", "放映厅:", QLineEdit::Normal, QString(), &ok); if (!ok) return;
    int capacity = QInputDialog::getInt(this, "新增", "座位容量:", 100, 1, 100000, 1, &ok); if (!ok) return;

    QSqlQuery q(m_model->database());
    q.prepare("INSERT INTO tickets (movieName,cinemaName,showDate,showTime,duration,price,hall,capacity,sold) VALUES (?,?,?,?,?,?,?,?,?)");
    q.addBindValue(movie);
    q.addBindValue(cinema);
    q.addBindValue(date);
    q.addBindValue(time);
    q.addBindValue(duration);
    q.addBindValue(price);
    q.addBindValue(hall);
    q.addBindValue(capacity);
    q.addBindValue(0);
    if (!q.exec()) QMessageBox::warning(this, "错误", q.lastError().text());
    refresh();
}

void MainWindow::onDelete()
{
    auto *view = qobject_cast<QTableView*>(centralWidget());
    if (!view) return;
    auto sel = view->selectionModel();
    if (!sel->hasSelection()) { QMessageBox::information(this, "提示", "请先选择一行"); return; }
    int row = sel->selectedRows().first().row();
    m_model->removeRow(row);
    m_model->submitAll();
    refresh();
}

void MainWindow::onSell()
{
    bool ok; int qty = QInputDialog::getInt(this, "售票", "数量:", 1, 1, 10000, 1, &ok); if (!ok) return;
    auto *view = qobject_cast<QTableView*>(centralWidget()); if (!view) return;
    if (!view->selectionModel()->hasSelection()) { QMessageBox::information(this, "提示", "请选择场次"); return; }
    int row = view->selectionModel()->selectedRows().first().row();
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();
    QSqlQuery q(m_model->database());
    q.prepare("UPDATE tickets SET sold = sold + ? WHERE id = ? AND sold + ? <= capacity");
    q.addBindValue(qty); q.addBindValue(id); q.addBindValue(qty);
    if (!q.exec() || q.numRowsAffected() == 0) QMessageBox::warning(this, "售票失败", "可能超出容量或发生错误");
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
