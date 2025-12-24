#pragma once
#include <QString>

struct Ticket {
    int id = -1;
    QString movieName;
    QString cinemaName;
    QString showDate; // YYYY-MM-DD
    QString showTime; // HH:MM
    int duration = 0; // minutes
    double price = 0.0;
    QString hall;
    int capacity = 0;
    int remain=0;
    int sold = 0;
    
    // 电影详情（合并后的单一字段）
    QString movieDetails;
};
