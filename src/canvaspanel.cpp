#include "canvaspanel.h"
#include <QSplitter>

CanvasPanel::CanvasPanel(CsvParser *parser, ScriptEngine *scriptEngine, QWidget *parent)
    : QWidget(parent)
    , m_csvParser(parser)
    , m_scriptEngine(scriptEngine)
{
    // 主布局使用分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->addWidget(splitter);
    
    // ========== 左侧列选择面板 ==========
    m_columnPanel = new QWidget();
    m_columnPanel->setMinimumWidth(180);
    m_columnPanel->setMaximumWidth(250);
    m_columnLayout = new QVBoxLayout(m_columnPanel);
    m_columnLayout->setContentsMargins(5, 5, 5, 5);
    m_columnLayout->setSpacing(8);
    
    // X轴选择
    QLabel *xAxisLabel = new QLabel("X轴数据源:");
    m_columnLayout->addWidget(xAxisLabel);
    
    m_xAxisComboBox = new QComboBox();
    m_xAxisComboBox->addItem("行索引", -1);
    connect(m_xAxisComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CanvasPanel::onXAxisChanged);
    m_columnLayout->addWidget(m_xAxisComboBox);
    
    // 列选择列表
    QLabel *yAxisLabel = new QLabel("Y轴数据 (点击添加/移除):");
    m_columnLayout->addWidget(yAxisLabel);
    
    m_columnListWidget = new QListWidget();
    m_columnListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    connect(m_columnListWidget, &QListWidget::itemClicked,
            this, &CanvasPanel::onColumnItemClicked);
    m_columnLayout->addWidget(m_columnListWidget, 1);
    
    // 清除按钮
    m_clearButton = new QPushButton("清除所有曲线");
    connect(m_clearButton, &QPushButton::clicked, this, &CanvasPanel::clearChart);
    m_columnLayout->addWidget(m_clearButton);
    
    // ========== 右侧图表 ==========
    m_chart = new ChartWidget();
    
    // 添加到分割器
    splitter->addWidget(m_columnPanel);
    splitter->addWidget(m_chart);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    // 初始化列列表
    refreshColumnList();
}

CanvasPanel::~CanvasPanel()
{
}

void CanvasPanel::refreshColumnList()
{
    // 暂时断开信号，避免在更新过程中触发
    m_xAxisComboBox->blockSignals(true);
    
    m_columnListWidget->clear();
    m_xAxisComboBox->clear();
    m_xAxisComboBox->addItem("行索引", -1);
    m_columnIndexMap.clear();
    m_selectedColumns.clear();
    m_selectedComputedColumns.clear();
    m_chart->clearChart();
    
    if (!m_csvParser || m_csvParser->getColumnCount() == 0) {
        m_xAxisComboBox->blockSignals(false);
        return;
    }
    
    QStringList columns = m_csvParser->getColumnNames();
    
    for (int i = 0; i < columns.size(); ++i) {
        QString colName = columns[i];
        bool isNumeric = m_csvParser->isNumericColumn(i);
        
        m_columnIndexMap[colName] = i;
        
        // 添加到Y轴列表
        QListWidgetItem *item = new QListWidgetItem(colName);
        item->setData(Qt::UserRole, i);  // 存储列索引
        item->setData(Qt::UserRole + 1, false);  // 标记为非计算列
        
        if (!isNumeric) {
            item->setForeground(Qt::gray);
            item->setToolTip("非数值列，无法用于绘图");
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        } else {
            item->setToolTip("点击添加到图表 / 再次点击移除");
            item->setCheckState(Qt::Unchecked);
        }
        m_columnListWidget->addItem(item);
        
        // 添加到X轴下拉框
        if (isNumeric) {
            m_xAxisComboBox->addItem(colName, i);
        }
    }
    
    // 添加计算列到列表和X轴下拉框
    for (auto it = m_computedColumns.constBegin(); it != m_computedColumns.constEnd(); ++it) {
        QListWidgetItem *item = new QListWidgetItem("📊 " + it.key());
        item->setData(Qt::UserRole, -1);  // 计算列使用-1作为索引
        item->setData(Qt::UserRole + 1, true);  // 标记为计算列
        item->setData(Qt::UserRole + 2, it.key());  // 存储计算列名称
        item->setToolTip("计算列: " + it.key() + " (点击添加/移除)");
        item->setCheckState(Qt::Unchecked);
        item->setForeground(QColor(0, 128, 0));  // 绿色显示计算列
        m_columnListWidget->addItem(item);
        
        // 添加计算列到X轴下拉框
        m_xAxisComboBox->addItem("📊 " + it.key(), QVariant::fromValue(QString("computed:" + it.key())));
    }
    
    // 恢复信号
    m_xAxisComboBox->blockSignals(false);
    
    // 更新Y轴列表的可选状态
    updateYAxisAvailability();
}

void CanvasPanel::clearChart()
{
    m_chart->clearChart();
    m_selectedColumns.clear();
    m_selectedComputedColumns.clear();
    
    // 重置所有复选框状态
    for (int i = 0; i < m_columnListWidget->count(); ++i) {
        QListWidgetItem *item = m_columnListWidget->item(i);
        if (item->flags() & Qt::ItemIsEnabled) {
            item->setCheckState(Qt::Unchecked);
        }
    }
}

void CanvasPanel::setTitle(const QString &title)
{
    m_chart->setChartTitle(title);
}

void CanvasPanel::onColumnItemClicked(QListWidgetItem *item)
{
    if (!(item->flags() & Qt::ItemIsEnabled)) {
        return;  // 非数值列，忽略
    }
    
    bool isComputed = item->data(Qt::UserRole + 1).toBool();
    
    if (isComputed) {
        // 计算列处理
        QString computedName = item->data(Qt::UserRole + 2).toString();
        if (m_selectedComputedColumns.contains(computedName)) {
            m_selectedComputedColumns.remove(computedName);
            item->setCheckState(Qt::Unchecked);
        } else {
            m_selectedComputedColumns.insert(computedName);
            item->setCheckState(Qt::Checked);
        }
    } else {
        // CSV列处理
        int columnIndex = item->data(Qt::UserRole).toInt();
        
        // 切换选中状态
        if (m_selectedColumns.contains(columnIndex)) {
            // 移除该列
            m_selectedColumns.remove(columnIndex);
            item->setCheckState(Qt::Unchecked);
        } else {
            // 添加该列
            m_selectedColumns.insert(columnIndex);
            item->setCheckState(Qt::Checked);
        }
    }
    
    // 更新图表
    updateChart();
}

void CanvasPanel::onXAxisChanged(int index)
{
    Q_UNUSED(index);
    
    // 更新Y轴列表的可选状态
    updateYAxisAvailability();
    
    // X轴变化时重新绘制，但只在有数据时
    if (m_csvParser && m_csvParser->getRowCount() > 0 && 
        (!m_selectedColumns.isEmpty() || !m_selectedComputedColumns.isEmpty())) {
        updateChart();
    }
}

void CanvasPanel::updateChart()
{
    m_chart->clearChart();
    
    if ((m_selectedColumns.isEmpty() && m_selectedComputedColumns.isEmpty()) || !m_csvParser) {
        return;
    }
    
    // 获取X轴数据
    QVector<double> xData = getXAxisData();
    QString xAxisLabel = m_xAxisComboBox->currentText();
    QString xAxisColumnName = getXAxisColumnName();
    bool xAxisIsComputed = isXAxisComputed();
    
    // 添加选中的CSV列
    QStringList columnNames = m_csvParser->getColumnNames();
    for (int colIndex : m_selectedColumns) {
        QString colName = columnNames[colIndex];
        
        // 跳过X轴列（如果X轴使用的是CSV列）
        if (!xAxisIsComputed && colName == xAxisColumnName) {
            continue;
        }
        
        QVector<double> yData = m_csvParser->getColumnData(colIndex);
        m_chart->addSeries(colName, xData, yData);
    }
    
    // 添加选中的计算列
    for (const QString &computedName : m_selectedComputedColumns) {
        // 跳过X轴列（如果X轴使用的是计算列）
        if (xAxisIsComputed && computedName == xAxisColumnName) {
            continue;
        }
        
        if (m_computedColumns.contains(computedName)) {
            QVector<double> yData = m_computedColumns[computedName];
            // 使用X轴数据或者生成对应长度的索引
            QVector<double> computedXData;
            if (xData.size() == yData.size()) {
                computedXData = xData;
            } else {
                // 如果长度不匹配，生成索引作为X轴
                for (int i = 0; i < yData.size(); ++i) {
                    computedXData.append(static_cast<double>(i));
                }
            }
            m_chart->addSeries(computedName, computedXData, yData);
        }
    }
    
    m_chart->setXAxisLabel(xAxisLabel);
    m_chart->setYAxisLabel("Value");
}

QVector<double> CanvasPanel::getXAxisData()
{
    QVector<double> xData;
    
    if (!m_csvParser || m_csvParser->getRowCount() == 0) {
        return xData;
    }
    
    QVariant xAxisData = m_xAxisComboBox->currentData();
    QString dataStr = xAxisData.toString();
    
    // 检查是否是计算列（存储为 "computed:列名" 格式）
    if (dataStr.startsWith("computed:")) {
        QString computedName = dataStr.mid(9);  // 去掉 "computed:" 前缀
        if (m_computedColumns.contains(computedName)) {
            return m_computedColumns[computedName];
        }
        // 计算列不存在，返回空
        return xData;
    }
    
    int xAxisIndex = xAxisData.toInt();
    
    if (xAxisIndex < 0) {
        // 使用行索引
        for (int i = 0; i < m_csvParser->getRowCount(); ++i) {
            xData.append(static_cast<double>(i));
        }
    } else {
        // 使用选定的CSV列
        xData = m_csvParser->getColumnData(xAxisIndex);
    }
    
    return xData;
}

QString CanvasPanel::getXAxisColumnName() const
{
    if (m_xAxisComboBox->currentIndex() == 0) {
        return "";  // 行索引
    }
    
    QVariant xAxisData = m_xAxisComboBox->currentData();
    QString dataStr = xAxisData.toString();
    
    // 检查是否是计算列
    if (dataStr.startsWith("computed:")) {
        return dataStr.mid(9);  // 返回计算列名称
    }
    
    // CSV列
    int xAxisIndex = xAxisData.toInt();
    if (m_csvParser && xAxisIndex >= 0 && xAxisIndex < m_csvParser->getColumnCount()) {
        return m_csvParser->getColumnNames()[xAxisIndex];
    }
    
    return "";
}

bool CanvasPanel::isXAxisComputed() const
{
    QVariant xAxisData = m_xAxisComboBox->currentData();
    return xAxisData.toString().startsWith("computed:");
}

void CanvasPanel::updateYAxisAvailability()
{
    QString xAxisName = getXAxisColumnName();
    bool xAxisIsComputed = isXAxisComputed();
    
    for (int i = 0; i < m_columnListWidget->count(); ++i) {
        QListWidgetItem *item = m_columnListWidget->item(i);
        bool isComputed = item->data(Qt::UserRole + 1).toBool();
        
        QString itemName;
        if (isComputed) {
            itemName = item->data(Qt::UserRole + 2).toString();
        } else {
            int colIndex = item->data(Qt::UserRole).toInt();
            if (m_csvParser && colIndex >= 0 && colIndex < m_csvParser->getColumnCount()) {
                itemName = m_csvParser->getColumnNames()[colIndex];
            }
        }
        
        // 检查是否是X轴使用的列
        bool isXAxisColumn = (itemName == xAxisName) && !xAxisName.isEmpty();
        
        // 检查原始是否是数值列（对于CSV列）
        bool originallyEnabled = true;
        if (!isComputed) {
            int colIndex = item->data(Qt::UserRole).toInt();
            if (m_csvParser && colIndex >= 0) {
                originallyEnabled = m_csvParser->isNumericColumn(colIndex);
            }
        }
        
        if (isXAxisColumn) {
            // X轴使用的列，禁用并取消选中
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            item->setForeground(Qt::gray);
            item->setToolTip("该列已被用作X轴，无法作为Y轴");
            
            // 如果之前选中了，需要取消选中
            if (isComputed) {
                if (m_selectedComputedColumns.contains(itemName)) {
                    m_selectedComputedColumns.remove(itemName);
                    item->setCheckState(Qt::Unchecked);
                }
            } else {
                int colIndex = item->data(Qt::UserRole).toInt();
                if (m_selectedColumns.contains(colIndex)) {
                    m_selectedColumns.remove(colIndex);
                    item->setCheckState(Qt::Unchecked);
                }
            }
        } else if (originallyEnabled) {
            // 恢复可用状态
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
            if (isComputed) {
                item->setForeground(QColor(0, 128, 0));  // 绿色
                item->setToolTip("计算列: " + itemName + " (点击添加/移除)");
            } else {
                item->setForeground(Qt::black);
                item->setToolTip("点击添加到图表 / 再次点击移除");
            }
        }
    }
}

PlotPreset CanvasPanel::getPreset() const
{
    PlotPreset preset;
    
    // 获取X轴列名
    if (m_xAxisComboBox->currentIndex() == 0) {
        preset.xAxisColumn = "";  // 使用行索引
    } else {
        preset.xAxisColumn = m_xAxisComboBox->currentText();
    }
    
    // 获取选中的Y轴列名
    if (m_csvParser) {
        QStringList columnNames = m_csvParser->getColumnNames();
        for (int colIndex : m_selectedColumns) {
            if (colIndex >= 0 && colIndex < columnNames.size()) {
                preset.yAxisColumns.append(columnNames[colIndex]);
            }
        }
    }
    
    // 获取选中的计算列
    for (const QString &colName : m_selectedComputedColumns) {
        preset.computedColumns.append(colName);
    }
    
    return preset;
}

void CanvasPanel::applyPreset(const PlotPreset &preset)
{
    if (!m_csvParser || m_csvParser->getColumnCount() == 0) {
        return;
    }
    
    // 先清除当前图表
    clearChart();
    
    // 设置X轴
    if (preset.xAxisColumn.isEmpty()) {
        m_xAxisComboBox->setCurrentIndex(0);  // 行索引
    } else {
        int index = m_xAxisComboBox->findText(preset.xAxisColumn);
        if (index >= 0) {
            m_xAxisComboBox->setCurrentIndex(index);
        }
    }
    
    // 设置Y轴列
    QStringList columnNames = m_csvParser->getColumnNames();
    for (const QString &colName : preset.yAxisColumns) {
        int colIndex = columnNames.indexOf(colName);
        if (colIndex >= 0 && m_csvParser->isNumericColumn(colIndex)) {
            m_selectedColumns.insert(colIndex);
            
            // 更新列表项的复选框状态
            for (int i = 0; i < m_columnListWidget->count(); ++i) {
                QListWidgetItem *item = m_columnListWidget->item(i);
                if (item->data(Qt::UserRole).toInt() == colIndex && 
                    !item->data(Qt::UserRole + 1).toBool()) {
                    item->setCheckState(Qt::Checked);
                    break;
                }
            }
        }
    }
    
    // 设置计算列
    for (const QString &colName : preset.computedColumns) {
        if (m_computedColumns.contains(colName)) {
            m_selectedComputedColumns.insert(colName);
            
            // 更新列表项的复选框状态
            for (int i = 0; i < m_columnListWidget->count(); ++i) {
                QListWidgetItem *item = m_columnListWidget->item(i);
                if (item->data(Qt::UserRole + 1).toBool() && 
                    item->data(Qt::UserRole + 2).toString() == colName) {
                    item->setCheckState(Qt::Checked);
                    break;
                }
            }
        }
    }
    
    // 更新图表
    updateChart();
}

void CanvasPanel::addComputedColumn(const QString &name, const QVector<double> &data)
{
    // 只更新内部数据存储，不添加到UI列表
    // UI列表的更新由 refreshColumnList() 统一处理
    m_computedColumns[name] = data;
}

void CanvasPanel::removeComputedColumn(const QString &name)
{
    // 只更新内部数据存储
    m_computedColumns.remove(name);
    m_selectedComputedColumns.remove(name);
    // UI更新由 refreshColumnList() 统一处理
}

void CanvasPanel::clearComputedColumns()
{
    m_computedColumns.clear();
    m_selectedComputedColumns.clear();
    // UI更新由 refreshColumnList() 统一处理
}
