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
#include <qmessagebox.h>
#include <qsqlquery.h>
#include <QDateTime>
#include "../model/database.h"

MainWindow::MainWindow(Database &db, TicketController &controller, QWidget *parent)
    : QMainWindow(parent)                              
    , m_ctrl(controller)                           
    , m_model(new QSqlTableModel(this, db.db()))      
    , m_db(db)                                       
{
    //数据模型设置
    m_model->setTable("tickets");      // 设置模型关联数据库中的"tickets"表
    m_model->select();                 // 从数据库加载数据到模型
    qDebug() << "数据库表字段列表:";
    // 正确的调试代码
    for (int i = 0; i < m_model->columnCount(); ++i) {
        qDebug() << i << ":" 
                << m_model->headerData(i, Qt::Horizontal).toString();
    }
    qDebug() << "数据库文件位置:" << db.db().databaseName();

    //表格视图创建与设置
    auto *view = new QTableView(this);                 // 创建表格视图
    view->setModel(m_model);                           // 设置视图的数据模型
    view->setSelectionBehavior(QAbstractItemView::SelectRows);     // 设置整行选择模式
    view->setSelectionMode(QAbstractItemView::SingleSelection);    // 设置单选模式
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch); // 表头自适应宽度
    view->verticalHeader()->setVisible(false);         // 隐藏行号列
    view->setAlternatingRowColors(true);               // 启用交替行颜色（斑马线效果）
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 设置大小策略为可扩展
    view->verticalHeader()->setDefaultSectionSize(36); // 设置行高为36像素
    
    QFont f = view->font(); 
    f.setPointSize(10); 
    view->setFont(f); // 设置表格字体大小为10
    setCentralWidget(view);                            // 将表格视图设置为主窗口的中心部件

    // UI列隐藏
    // 隐藏内部ID列（不显示给用户看，但数据模型中仍存在）
    int idCol = m_model->fieldIndex("id");             // 获取"id"字段在模型中的列索引
    if (idCol >= 0) view->hideColumn(idCol);           // 如果找到ID列，则隐藏它

    // 左侧功能边栏创建
    m_sideDock = new QDockWidget(tr("功能"), this);    // 创建可停靠窗口（边栏）
    m_sideDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea); // 允许停靠在左右两侧
    QWidget *sideWidget = new QWidget(this);           // 创建边栏内容部件
    QVBoxLayout *sideLayout = new QVBoxLayout(sideWidget); // 使用垂直布局管理器

    //统一创建边栏按钮
    auto addButton = [&](const QString &text, const QIcon &icon, const char *slot){
        QPushButton *b = new QPushButton(icon, text, this); // 创建带图标和文字的按钮
        b->setIconSize(QSize(20,20));                  // 设置图标大小
        b->setMinimumHeight(38);                       // 设置按钮最小高度
        b->setCursor(Qt::PointingHandCursor);          // 设置鼠标悬停为手指形状
        sideLayout->addWidget(b);                      // 将按钮添加到布局
        connect(b, SIGNAL(clicked()), this, slot);     // 连接按钮点击信号到对应槽函数
        return b;                                      // 返回按钮指针
    };

    // 功能按钮添加
    QStyle *st = QApplication::style();                // 获取应用程序样式（备用）
    
    // 核心功能按钮组
    addButton(tr("新增"), QIcon(":/icons/icons/add.svg"), SLOT(onAdd()));      // 新增场次
    addButton(tr("删除"), QIcon(":/icons/icons/delete.svg"), SLOT(onDelete())); // 删除场次
    addButton(tr("售票"), QIcon(":/icons/icons/sell.svg"), SLOT(onSell()));     // 售票功能
    addButton(tr("退票"), QIcon(":/icons/icons/refund.svg"), SLOT(onRefund())); // 退票功能
    sideLayout->addSpacing(8);// 添加8像素间距
    
    // 数据管理按钮组
    addButton(tr("导入"), QIcon(":/icons/icons/import.svg"), SLOT(onImport())); // 导入数据
    addButton(tr("导出"), QIcon(":/icons/icons/export.svg"), SLOT(onExport())); // 导出数据
    sideLayout->addSpacing(8);                         // 添加8像素间距
    
    // 查询与排序按钮组
    m_searchBtn = addButton(tr("查找"), QIcon(":/icons/icons/search.svg"), SLOT(onSearch())); // 查找功能
    addButton(tr("排序"), QIcon(":/icons/icons/sort.svg"), SLOT(onSort()));     // 排序功能

    sideLayout->addStretch();                          // 添加弹性空间，使后续按钮靠底部

    // 主题切换按钮
    QPushButton *themeBtn = new QPushButton(QIcon(":/icons/icons/theme.svg"), tr("切换主题"), this);
    themeBtn->setMinimumHeight(36);                    // 设置主题按钮最小高度
    connect(themeBtn, &QPushButton::clicked, this, &MainWindow::onToggleTheme); // 连接主题切换信号
    sideLayout->addWidget(themeBtn);                   // 将主题按钮添加到布局底部

    // 边栏完成设置
    sideWidget->setLayout(sideLayout);                 // 为边栏部件设置布局
    m_sideDock->setWidget(sideWidget);                 // 将边栏部件设置到停靠窗口
    addDockWidget(Qt::LeftDockWidgetArea, m_sideDock); // 将边栏停靠在主窗口左侧

    // 设置停靠窗口特性：可关闭、可移动、可浮动
    m_sideDock->setFeatures(QDockWidget::DockWidgetClosable | 
                           QDockWidget::DockWidgetMovable | 
                           QDockWidget::DockWidgetFloatable);
    
    // 工具栏设置
    // 创建工具栏动作用于显示/隐藏边栏
    m_toggleDockAction = new QAction(tr("功能"), this);
    m_toggleDockAction->setCheckable(true);                     // 设置为可勾选（切换状态）
    m_toggleDockAction->setChecked(m_sideDock->isVisible());    // 初始状态与边栏可见性同步
    connect(m_toggleDockAction, &QAction::toggled, m_sideDock, &QWidget::setVisible); // 动作触发边栏显示/隐藏
    connect(m_sideDock, &QDockWidget::visibilityChanged, m_toggleDockAction, &QAction::setChecked); // 边栏变化同步动作状态
    
    QToolBar *tb = addToolBar(tr("主工具"));          // 创建主工具栏
    tb->setMovable(false);                            // 禁止工具栏移动
    tb->addAction(m_toggleDockAction);                // 将边栏切换动作添加到工具栏

    // 主窗口基本设置
    setWindowTitle("电影票管理");                      // 设置窗口标题
    statusBar()->showMessage("就绪");                  // 在状态栏显示就绪消息
    setMinimumSize(1100, 600);                         // 设置窗口大小

    // 主题切换动画效果
    // 创建淡入淡出动画效果用于主题切换时的平滑过渡
    m_opEffect = new QGraphicsOpacityEffect(this);    // 创建透明度效果
    m_opEffect->setOpacity(1.0);                      // 初始完全不透明
    this->setGraphicsEffect(m_opEffect);              // 将效果应用到主窗口
    
    m_fadeAnim = new QPropertyAnimation(m_opEffect, "opacity", this); // 创建属性动画
    m_fadeAnim->setDuration(220);                     // 设置动画持续时间220毫秒
    connect(m_fadeAnim, &QPropertyAnimation::finished, this, &MainWindow::onFadeFinished); // 连接动画完成信号

    // 主题持久化设置
    // 使用QSettings保存用户主题偏好
    m_settings = new QSettings("CinemaOrder", "CinemaApp", this); // 创建设置对象
    m_darkTheme = m_settings->value("ui/dark", false).toBool();   // 从设置读取主题偏好，默认浅色主题
    applyTheme(m_darkTheme);                            // 立即应用保存的主题（无动画）
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
    q.prepare("INSERT INTO tickets (movieName,cinemaName,showDate,showTime,duration,price,hall,capacity,remain,sold) VALUES (?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(t.movieName);
    q.addBindValue(t.cinemaName);
    q.addBindValue(t.showDate);
    q.addBindValue(t.showTime);
    q.addBindValue(t.duration);
    q.addBindValue(t.price);
    q.addBindValue(t.hall);
    q.addBindValue(t.capacity);
    q.addBindValue(t.capacity-t.sold);
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

//售票槽函数
void MainWindow::onSell()
{
    // 获取主窗口中的表格视图控件
    auto *view = qobject_cast<QTableView*>(centralWidget()); 
    if (!view) return;  // 如果无法获取视图，直接返回
    
    // 检查用户是否在表格中选择了某一行
    if (!view->selectionModel()->hasSelection()) { 
        // 如果没有选中任何行，显示提示信息并返回
        QMessageBox::information(this, "提示", "请选择场次"); 
        return; 
    }
    
    // 获取选中行的行号（用户选择的场次所在的行）
    int row = view->selectionModel()->selectedRows().first().row();
    
    // 从数据模型中获取选中行的各个字段值
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();           // 场次ID
    int capacity = m_model->data(m_model->index(row, m_model->fieldIndex("capacity"))).toInt(); // 场次总座位数
    int sold = m_model->data(m_model->index(row, m_model->fieldIndex("sold"))).toInt();       // 已售座位数
    
    // 检查该场次是否已售罄
    if (sold >= capacity) { 
        QMessageBox::information(this, "提示", "该场次已售罄"); 
        return; 
    }

    //检查是否在可销售时间段
    QDateTime currentDateTime = QDateTime::currentDateTime();
    if(currentDateTime.date() > m_model->data(m_model->index(row, m_model->fieldIndex("showDate"))).toDate() ||
       (currentDateTime.date() == m_model->data(m_model->index(row, m_model->fieldIndex("showDate"))).toDate() &&
        currentDateTime.time() > m_model->data(m_model->index(row, m_model->fieldIndex("showTime"))).toTime())){
        QMessageBox::information(this, "提示", "该场次已结束，无法售票");
        return;
    }

    // 显示座位选择对话框，让用户选择具体的座位
    // 参数：数据库连接、场次ID、总座位数
    SeatSelectionDialog dlg(m_db, id, capacity,0,this);
    
    // 如果用户取消选择（点击取消按钮），则直接返回
    if (dlg.exec() != QDialog::Accepted) return;

    // 获取用户在对话框中选择的座位列表
    auto seats = dlg.selectedSeats();
    
    //没有选择任何座位，直接返回
    if (seats.isEmpty()) return;

    // 计算要售出的座位数量
    int toSell = seats.size();
    
    // 验证选择的座位数是否超过剩余座位数
    if (sold + toSell > capacity) { 
        QMessageBox::warning(this, "售票失败", "选择的座位超过剩余票数"); 
        return; 
    }

    // 开始数据库事务处理，确保数据一致性
    // 获取数据库连接
    QSqlDatabase db = m_model->database();
    
    // 开启事务：接下来的所有数据库操作要么全部成功，要么全部失败
    db.transaction();
    
    // 创建第一个SQL查询将选中的座位状态更新为status=0
    QSqlQuery q(db);
    q.prepare("UPDATE seats SET status = 1 WHERE id = ?");  // ? 是参数占位符
    
    // 遍历用户选择的每一个座位
    for (const SeatInfo &s : seats) {
        q.addBindValue(s.id);// 绑定座位ID参数
        if (!q.exec())
            qWarning() << "更新座位状态失败:" << q.lastError().text();  // 记录错误日志
        q.finish();// 结束当前查询，准备执行下一个
    }

    // 创建第二个SQL查询：更新票务表中的已售座位数
    QSqlQuery q2(db);

    // 准备SQL更新语句：增加已售票数
    q2.prepare("UPDATE tickets SET sold = sold+?,remain=remain-? WHERE id = ? AND sold+?<=capacity");
    q2.addBindValue(toSell);   
    q2.addBindValue(toSell);
    q2.addBindValue(id);       
    q2.addBindValue(toSell);  // 补缺：第 4 个参数绑定
    
    // 执行更新操作
    bool ok = q2.exec();
    
    //检查更新是否成功
    if (!ok) {
        //更新失败，回滚整个事务
        db.rollback();
        QMessageBox::warning(this, "售票失败", "更新票务信息失败");
    } else {
        // 如果所有操作都成功，永久保存所有更改
        db.commit();
    }
    
    //刷新表格视图，显示最新的数据
    refresh();
}

//退票槽函数
void MainWindow::onRefund()
{
    // 获取主窗口中的表格视图控件
    auto *view = qobject_cast<QTableView*>(centralWidget()); 
    
    // 如果无法获取表格视图，直接返回
    if (!view) return;
    
    // 检查用户是否在表格中选择了某一行（即选择了某个电影场次）
    if (!view->selectionModel()->hasSelection()) { 
        // 如果没有选中任何行，显示提示信息并返回
        QMessageBox::information(this, "提示", "请选择场次"); 
        return; 
    }
    
    int row = view->selectionModel()->selectedRows().first().row();// 获取选中行的行号（用户选择的场次所在的行）
    int id = m_model->data(m_model->index(row, m_model->fieldIndex("id"))).toInt();// 从数据模型中获取选中行的场次ID
    int capacity = m_model->data(m_model->index(row, m_model->fieldIndex("capacity"))).toInt(); // 场次总座位数
    int sold = m_model->data(m_model->index(row, m_model->fieldIndex("sold"))).toInt();       // 已售座位数
    
    if (sold==0) { 
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

    SeatSelectionDialog dlg(m_db, id, capacity,1,this);
    if (dlg.exec() != QDialog::Accepted) return;
    
    QSqlDatabase db = m_model->database();

    auto seats = dlg.selectedSeats();

    if(seats.isEmpty()) return;

    int toRefund = seats.size();

    if (sold-toRefund < 0) { 
        QMessageBox::warning(this, "退票失败", "选择的座位超过已售票数"); 
        return; 
    }
    
    // 开启事务：接下来的所有数据库操作要么全部成功，要么全部失败
    db.transaction();
    QSqlQuery q1(db);
    
    q1.prepare("UPDATE seats SET status=0 WHERE id = ? AND status=1");
    
    
    // 执行SQL更新语句，并检查执行结果
    for(const auto &s:seats){
        q1.addBindValue(s.id);
        if(!q1.exec()){
            qWarning() << "退票失败:" << q1.lastError().text();
        }
        q1.finish();
    }
    
    //准备sql语句二
    QSqlQuery q2(db);
    q2.prepare("UPDATE tickets SET sold=sold-?, remain=remain+? WHERE id=? AND sold-?>=0");

    q2.addBindValue(toRefund);
    q2.addBindValue(toRefund);
    q2.addBindValue(id);
    q2.addBindValue(toRefund);
    // 无论成功或失败，都刷新表格视图，显示最新的数据

    bool ok = q2.exec();
    if(!ok){
        db.rollback();
        QMessageBox::warning(this, "退票失败", "更新票务信息失败");
    }else{
        db.commit();
    }
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
}}

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
