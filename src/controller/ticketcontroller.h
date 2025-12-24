#pragma once
#include "../model/ticket.h"
#include "../model/database.h"
#include <QList>
#include <QString>

// 票务业务逻辑控制器
class TicketController {
public:
    explicit TicketController(Database &db);
    // 新增票务
    bool addTicket(const Ticket &t);
    // 按条件查询票务
    QList<Ticket> findTickets(const QString &movie = QString(), const QString &cinema = QString(), const QString &date = QString(), const QString &time = QString());
    // 获取所有票务
    QList<Ticket> allTickets();
    // 获取排序后的票务列表
    QList<Ticket> listAll(const QString &orderBy = QString());
    // 删除指定票务
    bool deleteTicket(int id);
    // 售出指定数量的票
    bool sellTickets(int id, int qty);
    // 退回指定数量的票
    bool refundTickets(int id, int qty);
    // 导出到文件
    bool saveToFile(const QString &path);
    // 导入文件数据
    bool loadFromFile(const QString &path, bool clearBefore = false);
private:
    Database &m_db;     // 数据库实例
};

