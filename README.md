# CinemaOrder — 电影票管理（Qt6 + CMake）

快速说明：控制台应用，使用 Qt6 Core + Qt6 Sql（SQLite）实现 MVC 风格的电影票管理。

构建（Windows 示例，假设已安装 Qt6 并配置好环境）:

```powershell
mkdir build
cd build
cmake .. -G "NMake Makefiles" -DCMAKE_PREFIX_PATH="<Qt6安装路径>\lib\cmake\Qt6"
cmake --build .
```

运行：

```powershell
.\cinema_order.exe
```

功能：新增/查找/删除/排序/售票/退票/导入/导出等，数据保存在 `tickets.db`（SQLite）。

后续：可接入 AI agent（例如添加自然语言检索、智能推荐或自动化售票脚本）。
