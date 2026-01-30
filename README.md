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

### Windows (使用MinGW)

```batch
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

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
