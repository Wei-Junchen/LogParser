# LogParser - CSV数据可视化工具

一个基于C++和Qt的CSV文件数据可视化工具，支持多列数据选择和灵活的绘图模式。

## 功能特性

- 📂 **CSV文件读取**：支持打开和解析CSV格式文件
- 📊 **多列数据选择**：可从文件中选择多个数值列进行绘图
- 🎨 **灵活绘图模式**：
  - 单图模式：将所有选中的列绘制在同一张图表上
  - 多图模式：每个选中的列单独绘制一张图表
- 📐 **X轴数据源选择**：可选择使用行索引或某一数值列作为X轴
- 🔍 **图表交互**：支持鼠标拖拽缩放和平移
- 💾 **图表导出**：支持将图表保存为PNG/JPEG图片
- 🏷️ **多标签页管理**：支持创建多个图表标签页

## 系统要求

- Qt 5.15+ 或 Qt 6.x
- CMake 3.16+
- C++17兼容的编译器

### 依赖库

- Qt Widgets
- Qt Charts
- Qt QML

## 环境配置

### Ubuntu / Debian（推荐 Qt 6）

> 注意：在 Ubuntu 上，Qt Charts 的开发包名通常是 `libqt6charts6-dev`，不是 `qt6-charts-dev`。

```bash
# 1) 安装基础工具
sudo apt update
sudo apt install -y build-essential cmake

# 2) 安装 Qt6 依赖（本项目需要 Widgets + Charts + Qml）
sudo apt install -y qt6-base-dev qt6-declarative-dev libqt6charts6-dev

# 3) 验证依赖是否就绪（可选）
apt-cache policy qt6-base-dev qt6-declarative-dev libqt6charts6-dev
```

如果提示找不到上述包，先启用 `universe` 源再安装：

```bash
sudo add-apt-repository universe
sudo apt update
sudo apt install -y qt6-base-dev qt6-declarative-dev libqt6charts6-dev
```

## 编译指南

### Linux / macOS

```bash
# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 运行
./LogParser
```

### Windows (使用Visual Studio)

```batch
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Windows (使用MinGW) - 推荐方式 ⭐

#### 快速打包（一键生成安装程序）

只需一行命令即可完成从源码到可分发安装包的全过程：

```powershell
powershell -ExecutionPolicy Bypass -File .\package.ps1
```

或在 PowerShell 中直接运行：

```powershell
.\package.ps1
```

**脚本会自动执行以下步骤：**
1. ✅ 检测并编译项目（如未构建则自动编译）
2. ✅ 收集所有依赖库（Qt DLL、MinGW运行时库）
3. ✅ 配置 Qt 插件和资源
4. ✅ 使用 NSIS 生成 Windows 安装程序
5. ✅ 生成安装包：`LogParser_Setup.exe`

**前置要求：**
- Qt 6.5.3 MinGW 版本（安装在 `C:\Qt\6.5.3\mingw_64`）
- MinGW 工具链（安装在 `C:\Qt\Tools\mingw1120_64`）
- CMake 3.16+
- NSIS（Nullsoft Scriptable Install System）

**其他选项：**

```powershell
# 清理构建和安装包
.\package.ps1 -Clean

# 重新打包（保留已编译的二进制）
.\package.ps1
```

#### 手动构建（如果不需要打包）

```batch
mkdir build-mingw
cd build-mingw
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\mingw_64"
cmake --build . --config Release -j
cd ..
build-mingw\LogParser.exe
```

#### Windows 安装程序使用

安装包 `LogParser_Setup.exe` 支持以下功能：
- 安装到自定义位置（默认 `C:\Program Files\LogParser`）
- 创建桌面快捷方式
- 创建开始菜单快捷方式
- 完全卸载功能（通过控制面板）
- 以管理员身份运行

## 使用说明

1. **打开文件**：点击"打开CSV文件"按钮，选择要分析的CSV文件
2. **选择X轴**：从下拉框中选择X轴数据源（默认使用行索引）
3. **选择Y轴列**：在列表中点击选择一列或多列数值列（按Ctrl可多选）
4. **选择绘图模式**：
   - "将所有选中列绘制在同一张图表"：适合对比多组数据
   - "每列单独绘制一张图表"：适合详细查看单个数据序列
5. **绘制图表**：点击"绘制图表"按钮
6. **图表交互**：
   - 鼠标左键拖拽选择区域可缩放
   - 右键点击可重置视图
7. **保存图表**：点击"保存当前图表"可导出为图片

## 项目结构

```
LogParser/
├── CMakeLists.txt          # CMake配置文件
├── README.md               # 项目说明文档
├── src/
│   ├── main.cpp            # 程序入口
│   ├── mainwindow.h        # 主窗口头文件
│   ├── mainwindow.cpp      # 主窗口实现
│   ├── csvparser.h         # CSV解析器头文件
│   ├── csvparser.cpp       # CSV解析器实现
│   ├── chartwidget.h       # 图表组件头文件
│   └── chartwidget.cpp     # 图表组件实现
└── test_data/
    └── sample.csv          # 示例CSV数据文件
```

## 示例数据

项目包含一个示例CSV文件 `test_data/sample.csv`，包含以下列：
- Time（时间）
- Temperature（温度）
- Pressure（压力）
- Humidity（湿度）
- Speed（速度）
- Voltage（电压）

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| Ctrl+O | 打开文件 |
| Ctrl+S | 保存图表 |
| Ctrl+P | 绘制图表 |
| Ctrl+T | 新建图表标签页 |
| Ctrl+Q | 退出程序 |

## 许可证

MIT License
