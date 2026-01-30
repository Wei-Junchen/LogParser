#ifndef CANVASPANEL_H
#define CANVASPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QComboBox>
#include <QSet>
#include "chartwidget.h"
#include "csvparser.h"
#include "presetmanager.h"
#include "scriptengine.h"

/**
 * @brief Canvas面板类
 * 包含列选择列表和图表，点击列名直接添加/移除数据线
 */
class CanvasPanel : public QWidget
{
    Q_OBJECT
    
public:
    explicit CanvasPanel(CsvParser *parser, ScriptEngine *scriptEngine = nullptr, QWidget *parent = nullptr);
    ~CanvasPanel();
    
    /**
     * @brief 刷新列列表
     */
    void refreshColumnList();
    
    /**
     * @brief 清除图表
     */
    void clearChart();
    
    /**
     * @brief 获取图表组件
     */
    ChartWidget* getChart() const { return m_chart; }
    
    /**
     * @brief 设置Canvas标题
     */
    void setTitle(const QString &title);
    
    /**
     * @brief 获取当前Canvas的绘图预设
     */
    PlotPreset getPreset() const;
    
    /**
     * @brief 应用绘图预设
     */
    void applyPreset(const PlotPreset &preset);

private slots:
    /**
     * @brief 列项点击事件
     */
    void onColumnItemClicked(QListWidgetItem *item);
    
    /**
     * @brief X轴选择变化
     */
    void onXAxisChanged(int index);

private:
    /**
     * @brief 更新图表
     */
    void updateChart();
    
    /**
     * @brief 获取X轴数据
     */
    QVector<double> getXAxisData();
    
    /**
     * @brief 获取当前X轴列名
     */
    QString getXAxisColumnName() const;
    
    /**
     * @brief 检查X轴是否使用计算列
     */
    bool isXAxisComputed() const;
    
    /**
     * @brief 更新Y轴列表的可选状态
     * 当X轴选择某列时，该列在Y轴变为不可选
     */
    void updateYAxisAvailability();

public:
    /**
     * @brief 添加计算列（来自脚本引擎）
     * @param name 列名
     * @param data 数据
     */
    void addComputedColumn(const QString &name, const QVector<double> &data);
    
    /**
     * @brief 移除计算列
     * @param name 列名
     */
    void removeComputedColumn(const QString &name);
    
    /**
     * @brief 清除所有计算列
     */
    void clearComputedColumns();

private:
    CsvParser *m_csvParser;
    ScriptEngine *m_scriptEngine;
    
    // UI组件
    QHBoxLayout *m_mainLayout;
    
    // 左侧列选择面板
    QWidget *m_columnPanel;
    QVBoxLayout *m_columnLayout;
    QComboBox *m_xAxisComboBox;
    QListWidget *m_columnListWidget;
    QPushButton *m_clearButton;
    
    // 右侧图表
    ChartWidget *m_chart;
    
    // 已选中的列（用于绘图）
    QSet<int> m_selectedColumns;
    
    // 列名到索引的映射
    QMap<QString, int> m_columnIndexMap;
    
    // 计算列数据存储 (列名 -> 数据)
    QMap<QString, QVector<double>> m_computedColumns;
    
    // 已选中的计算列
    QSet<QString> m_selectedComputedColumns;
};

#endif // CANVASPANEL_H
