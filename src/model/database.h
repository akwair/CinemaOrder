#pragma once
#include <QtSql/QSqlDatabase>
#include <QString>

// 数据库管理类
class Database {
public:
    // 构造函数，参数为数据库文件路径
    explicit Database(const QString &path = QStringLiteral("tickets.db"));
    ~Database();
    // 打开数据库连接
    bool open();
    // 获取数据库实例
    QSqlDatabase db() const;
    // 初始化数据表
    bool ensureTables();
private:
    QString m_path;         // 数据库文件路径
    QSqlDatabase m_db;      // 数据库连接对象
};
