#pragma once
#include <QDialog>
#include <QVector>

class Database;

// 座位信息数据结构
struct SeatInfo {
    int id;                     // 座位表主键
    int row;                    // 座位行号
    int col;                    // 座位列号
    QString label;              // 座位标签(如A1)
    int status;                 // 座位状态(0可用 1已售)
    QString username;           // 购票用户
    QString userFullName;       // 用户姓名
    QString userPhoneNumber;    // 用户电话
    QString userEmail;          // 用户邮箱
};

// 座位选择对话框
class SeatSelectionDialog : public QDialog {
    Q_OBJECT
public:
    // flag: 0=售票 1=退票
    SeatSelectionDialog(Database &db, int ticketId, int capacity, int flag, const QString &username, bool isAdmin = false, QWidget *parent = nullptr);
    // 获取选中座位列表
    QVector<SeatInfo> selectedSeats() const { return m_selected; }
private slots:
    // 座位按钮点击处理
    void toggleSeat();
private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    // 初始化座位数据
    void ensureSeatsCreated(int ticketId, int capacity);
    // 加载座位数据
    void loadSeats(int ticketId);
    Database &m_db;              // 数据库
    int m_ticketId;              // 票务ID
    int m_flag;                  // 操作类型(0售 1退)
    QString m_username;          // 当前用户
    bool m_isAdmin;              // 是否管理员
    QVector<SeatInfo> m_seats;   // 所有座位
    QVector<SeatInfo> m_selected;// 选中座位
};
