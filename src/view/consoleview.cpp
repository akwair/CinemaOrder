
#include "consoleview.h"
#include "../controller/ticketcontroller.h"
#include "../model/ticket.h"
#include <QTextStream>
#include <QDebug>
#include <cstdio>

ConsoleView::ConsoleView(TicketController &controller)
    : m_ctrl(controller)
    , m_in(stdin)
    , m_out(stdout)
{
    // QTextStream encoding handled by console codepage; avoid setEncoding for portability
}

QString ConsoleView::readLine()
{
    QString s = m_in.readLine();
    return s.trimmed();
}

void ConsoleView::showTicket(const Ticket &t)
{
    m_out << "[" << t.id << "] "
          << t.movieName << " | "
          << t.cinemaName << " | "
          << t.showDate << " " << t.showTime << " | "
          << t.hall << " | "
          << t.price << " | sold:" << t.sold << "/" << t.capacity << "\n";
}

void ConsoleView::run()
{
    while (true) {
        m_out << "\n=== 电影票管理菜单 ===\n";
        m_out << "1. 新增电影场次\n";
        m_out << "2. 查找电影场次\n";
        m_out << "3. 删除电影场次\n";
        m_out << "4. 列出所有并排序\n";
        m_out << "5. 售票\n";
        m_out << "6. 退票\n";
        m_out << "7. 保存到文件\n";
        m_out << "8. 从文件载入\n";
        m_out << "0. 退出\n";
        m_out << "选择: ";
        m_out.flush();
        QString choice = readLine();
        if (choice == "0") break;
        if (choice == "1") {
            Ticket t;
            m_out << "电影名称: "; m_out.flush(); t.movieName = readLine();
            m_out << "影院名称: "; m_out.flush(); t.cinemaName = readLine();
            m_out << "放映日期 (YYYY-MM-DD): "; m_out.flush(); t.showDate = readLine();
            m_out << "放映时间 (HH:MM): "; m_out.flush(); t.showTime = readLine();
            m_out << "电影时长（分钟）: "; m_out.flush(); t.duration = readLine().toInt();
            m_out << "票价: "; m_out.flush(); t.price = readLine().toDouble();
            m_out << "放映厅: "; m_out.flush(); t.hall = readLine();
            m_out << "座位容量: "; m_out.flush(); t.capacity = readLine().toInt();
            t.sold = 0;
            if (m_ctrl.addTicket(t)) m_out << "新增成功\n"; else m_out << "新增失败\n";
        } else if (choice == "2") {
            m_out << "按电影名称查找（回车跳过）: "; m_out.flush(); QString name = readLine();
            m_out << "按影院名称查找（回车跳过）: "; m_out.flush(); QString cinema = readLine();
            m_out << "按放映日期（YYYY-MM-DD，回车跳过）: "; m_out.flush(); QString date = readLine();
            m_out << "按放映时间（HH:MM，回车跳过）: "; m_out.flush(); QString time = readLine();
            auto list = m_ctrl.findTickets(name, cinema, date, time);
            for (const Ticket &t : list) showTicket(t);
            m_out << "共 " << list.size() << " 条结果\n";
        } else if (choice == "3") {
            m_out << "先查找要删除的记录。输入关键字: "; m_out.flush(); QString key = readLine();
            auto list = m_ctrl.findTickets(key, key, QString(), QString());
            for (const Ticket &t : list) showTicket(t);
            if (list.isEmpty()) { m_out << "无匹配记录\n"; continue; }
            m_out << "输入要删除的记录 ID: "; m_out.flush(); int id = readLine().toInt();
            if (m_ctrl.deleteTicket(id)) m_out << "删除成功\n"; else m_out << "删除失败\n";
        } else if (choice == "4") {
            m_out << "排序依据（showDate/showTime/price），回车无序: "; m_out.flush(); QString ord = readLine();
            auto list = m_ctrl.listAll(ord);
            for (const Ticket &t : list) showTicket(t);
        } else if (choice == "5") {
            m_out << "输入场次 ID: "; m_out.flush(); int id = readLine().toInt();
            m_out << "售票张数: "; m_out.flush(); int q = readLine().toInt();
            if (m_ctrl.sellTickets(id, q)) m_out << "售票成功\n"; else m_out << "售票失败（超出容量或ID错误）\n";
        } else if (choice == "6") {
            m_out << "输入场次 ID: "; m_out.flush(); int id = readLine().toInt();
            m_out << "退票张数: "; m_out.flush(); int q = readLine().toInt();
            if (m_ctrl.refundTickets(id, q)) m_out << "退票成功\n"; else m_out << "退票失败（退票数非法或ID错误）\n";
        } else if (choice == "7") {
            m_out << "保存到文件路径（例如 tickets.csv）: "; m_out.flush(); QString path = readLine();
            if (m_ctrl.saveToFile(path)) m_out << "保存成功\n"; else m_out << "保存失败\n";
        } else if (choice == "8") {
            m_out << "从文件载入路径（例如 tickets.csv）: "; m_out.flush(); QString path = readLine();
            m_out << "载入前清空数据库？(y/N): "; m_out.flush(); QString ans = readLine();
            bool clear = (ans.toLower() == "y");
            if (m_ctrl.loadFromFile(path, clear)) m_out << "载入完成\n"; else m_out << "载入失败\n";
        } else {
            m_out << "无效选项\n";
        }
    }
}
