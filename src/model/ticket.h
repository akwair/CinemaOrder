#pragma once
#include <QString>

// 票务信息数据结构
struct Ticket {
    int id = -1;                    // 票务唯一标识
    QString movieName;              // 电影名称
    QString cinemaName;             // 影院名称
    QString showDate;               // 放映日期（格式：年-月-日）
    QString showTime;               // 放映时间（格式：时:分）
    int duration = 0;               // 电影时长(分钟)
    double price = 0.0;             // 票价
    QString hall;                   // 放映厅号
    int capacity = 0;               // 座位总数
    int remain=0;                   // 剩余座位数
    int sold = 0;                   // 已售座位数
    
    // 电影详情(合并字段)
    QString movieDetails;           // 导演、演员、类型等
};
