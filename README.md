# CinemaOrder — 电影票管理（Qt6 + CMake）

简要说明

- 这是一个基于 Qt6 的电影票管理示例程序，采用轻量 MVC 架构：
- 模型：src/model（`Ticket`、`Database`）
- 控制器：src/controller（`TicketController`）
- 视图：src/view（控制台 `ConsoleView`、GUI `MainWindow`、对话框等）
- 使用 `Qt6::Core` 与 `Qt6::Sql`（SQLite）保存数据，UI 使用 `Qt6::Widgets`（基于 `QSqlTableModel`）
- 图标资源放在 `resources/resources.qrc`（SVG），程序支持深/浅色主题切换并持久化设置

环境与依赖

- 推荐：Windows + MSYS2 (Mingw-w64) 或任意支持 Qt6 的平台
- 需安装：Qt6（包含 `Qt6::Core`, `Qt6::Sql`, `Qt6::Widgets`）、CMake、编译器（MinGW 或 MSVC）
- MSYS2 (MINGW64) 示例：
- `pacman -S mingw-w64-x86_64-qt6 mingw-w64-x86_64-toolchain`

项目关键文件

- `CMakeLists.txt`：工程配置、查找 Qt、AUTOMOC/AUTORCC/AUTOUIC、资源打包
- `src/main.cpp`：程序入口（登录对话后打开主窗口，可切回控制台模式）
- `src/model/database.{h,cpp}`：`QSqlDatabase` 封装与建表（`tickets`、`users`）
- `src/model/ticket.h`：`Ticket` 数据结构
- `src/controller/ticketcontroller.{h,cpp}`：票务增删改查、售票/退票、导入/导出
- `src/auth/authmanager.{h,cpp}`：简单认证（用户名 + SHA256(password)）
- `src/view/*`：GUI（`MainWindow`、`TicketFormDialog`、`LoginDialog`、`RegisterDialog`）与控制台视图
- `resources/resources.qrc` 与 `resources/icons/*.svg`：SVG 图标资源

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

常见问题与排查

- 链接失败（`permission denied`）：可执行文件被占用，先停止该进程再重建：
- PowerShell: `Get-Process cinema_order -ErrorAction SilentlyContinue | Stop-Process -Force`
- MOC/自动生成问题：已启用 `CMAKE_AUTOMOC`。若添加带 `Q_OBJECT` 的类但未被自动处理，可在 `CMakeLists.txt` 中使用 `qt6_wrap_cpp` 手动列出头文件。
- 终端中文乱码：Windows 上在 `main.cpp` 处设置控制台为 UTF-8（`SetConsoleOutputCP(CP_UTF8)`）；在 MSYS2 下使用 `export LANG=zh_CN.UTF-8`。

开发与修改指南

- 改 UI 布局/样式：编辑 `src/view/mainwindow.cpp` 与 `src/main.cpp` 中的样式字符串；主题持久化键为 `ui/dark`（`QSettings("CinemaOrder","CinemaApp")`）。
- 新增对话：头文件需带 `Q_OBJECT`，并确保被 CMake 的 AUTOMOC 或 `qt6_wrap_cpp` 覆盖。
- 资源：将 SVG 放到 `resources/icons/` 并更新 `resources/resources.qrc`，CMake 会调用 `qt6_add_resources` 自动生成资源源文件。
- 数据库文件位于构建目录（例如 `build/tickets.db`），修改表结构请考虑迁移或删除旧 DB 以便程序重建。

切换到控制台模式

- 若需在无 GUI 场景运行：修改 `src/main.cpp`，在登录后实例化并调用 `ConsoleView::run()` 替代 `MainWindow`。

扩展建议（AI Agent / API）

- 本地 IPC：使用 `QLocalServer`/`QLocalSocket` 供本地 agent 调用。
- HTTP API：嵌入轻量 HTTP 接口（`QTcpServer` 或第三方库）以暴露 REST 操作。
- CLI：添加 `--headless` 或子命令模式，供自动化脚本或 agent 使用。

