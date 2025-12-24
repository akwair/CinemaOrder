# CinemaOrder — 电影票管理系统

> 基于 Qt6 + CMake 的现代化电影票务管理系统

## 📋 项目概述

这是一个采用 MVC 架构设计的电影票管理系统，支持：
- ✅ 用户登录/注册（管理员/普通用户）
- ✅ 电影场次管理（增删改查）
- ✅ 电影详情编辑（导演、主演、类型、评分等）
- ✅ 座位选择与购票
- ✅ 退票功能
- ✅ 票务查询与统计
- ✅ 数据导入/导出
- ✅ 深色/浅色主题切换

## 🏗️ 项目结构

```
CinemaOrder/
├── src/                    # 源代码目录
│   ├── main.cpp           # 程序入口
│   ├── auth/              # 认证模块
│   │   ├── authmanager.h
│   │   └── authmanager.cpp
│   ├── model/             # 数据模型层
│   │   ├── database.h     # 数据库封装
│   │   ├── database.cpp
│   │   └── ticket.h       # 票务数据结构
│   ├── controller/        # 控制器层
│   │   ├── ticketcontroller.h
│   │   └── ticketcontroller.cpp
│   └── view/              # 视图层（UI）
│       ├── mainwindow.*          # 管理员主界面
│       ├── usermainwindow.*      # 用户主界面
│       ├── logindialog.*         # 登录对话框
│       ├── registerdialog.*      # 注册对话框
│       ├── moviedetaildialog.*   # 电影详情对话框
│       ├── seatselectiondialog.* # 座位选择对话框
│       ├── ticketformdialog.*    # 票务表单对话框
│       └── userinfodialog.*      # 用户信息对话框
├── resources/             # 资源文件
│   ├── resources.qrc      # Qt资源配置
│   └── icons/             # SVG图标
├── build/                 # 构建输出目录（自动生成）
├── CMakeLists.txt         # CMake配置文件
├── README.md              # 项目说明文档
└── LICENSE                # 许可证文件
```

## 🛠️ 技术栈

- **框架**: Qt6 (Core, Sql, Widgets)
- **数据库**: SQLite
- **构建工具**: CMake 3.16+
- **编译器**: MinGW-w64 / MSVC / GCC
- **架构模式**: MVC (Model-View-Controller)

快速构建（MSYS2 MINGW64）

```bash
cd /d/CinemaOrder
mkdir -p build
cd build
# 若 Qt 安装在 /mingw64，CMake 会自动查找；可显式指定 -DQT_ROOT=/mingw64
cmake .. -G "MinGW Makefiles" -DQT_ROOT=/mingw64
mingw32-make -j$(nproc)
```

Windows PowerShell（使用 MinGW Makefiles）

```powershell
cd D:\CinemaOrder
mkdir build; cd build
cmake .. -G "MinGW Makefiles" -DQT_ROOT=C:/msys64/mingw64
mingw32-make
```

运行

```powershell
cd build
.\cinema_order.exe
```

## ❓ 常见问题

### 编译问题

**Q: 链接失败 (permission denied)**
```bash
# Windows PowerShell
Get-Process cinema_order -ErrorAction SilentlyContinue | Stop-Process -Force

# 重新编译
mingw32-make -C build clean
mingw32-make -C build -j2
```

**Q: 找不到Qt库**
```bash
# 确保设置了正确的Qt路径
cmake -S . -B build -G "MinGW Makefiles" -DQT_ROOT=/path/to/qt6
```

**Q: MOC相关错误**
- 确保所有包含 `Q_OBJECT` 的类已在 CMakeLists.txt 中正确配置
- 清理build目录后重新构建

### 运行问题

**Q: 中文乱码**
```bash
# MSYS2环境
export LANG=zh_CN.UTF-8

# Windows控制台
chcp 65001
```

**Q: 找不到图标**
- 检查 `resources/resources.qrc` 是否正确配置
- 确保图标文件在 `resources/icons/` 目录下

**Q: 数据库文件位置**
- 数据库文件 `tickets.db` 默认在程序运行目录
- 可在代码中修改 `Database` 类的路径配置

## 📚 开发指南

### 代码规范

1. **命名约定**
   - 类名: PascalCase (`MainWindow`, `TicketController`)
   - 函数/变量: camelCase (`onAdd`, `movieName`)
   - 成员变量: m_ 前缀 (`m_model`, `m_db`)
   - 常量: 全大写 (`MAX_CAPACITY`)

2. **文件组织**
   - 每个类对应独立的 .h 和 .cpp 文件
   - 头文件使用 `#pragma once`
   - include 顺序: Qt库 → 标准库 → 项目内部

3. **注释规范**
   ```cpp
   /**
    * @brief 简要描述
    * @param param1 参数1说明
    * @return 返回值说明
    */
   ```

### 添加新功能

1. **添加新对话框**
```cpp
// 1. 创建头文件 src/view/mydialog.h
#pragma once
#include <QDialog>

class MyDialog : public QDialog {
    Q_OBJECT
public:
    explicit MyDialog(QWidget *parent = nullptr);
private:
    // 成员变量
};

// 2. 更新CMakeLists.txt
set(MOC_HEADERS
    ${CMAKE_SOURCE_DIR}/src/view/mydialog.h
    # ... 其他头文件
)
```

2. **添加数据库字段**
```cpp
// 在 database.cpp 的 ensureTables() 中添加
ALTER TABLE tickets ADD COLUMN new_field TEXT
```

3. **添加图标资源**
```xml
<!-- 在 resources/resources.qrc 中添加 -->
<file>icons/myicon.svg</file>
```

### 主题定制

编辑 `mainwindow.cpp` 中的 `applyTheme()` 函数：

```cpp
qApp->setStyleSheet(R"(
    QMainWindow { 
        background: #yourcolor; 
    }
    /* 更多样式 */
)");
```

## 🤝 贡献指南

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 🔗 相关链接

- [Qt6 文档](https://doc.qt.io/qt-6/)
- [CMake 文档](https://cmake.org/documentation/)
- [SQLite 文档](https://www.sqlite.org/docs.html)

## 💡 未来计划

- [ ] 添加数据统计图表
- [ ] 实现邮件通知功能
- [ ] 支持多语言界面
- [ ] 添加打印票据功能
- [ ] 实现在线支付接口
- [ ] 移动端应用支持

