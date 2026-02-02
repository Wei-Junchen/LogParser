#ifndef SERIESSTYLEDIALOG_H
#define SERIESSTYLEDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QJsonObject>

/**
 * @brief 曲线显示模式枚举
 */
enum class SeriesDisplayMode {
    Line,           // 连线模式（默认）
    Scatter,        // 散点模式
    LineAndScatter  // 连线+散点
};

/**
 * @brief 曲线样式结构
 * 保存一条曲线的显示设置
 */
struct SeriesStyle
{
    // 显示模式
    SeriesDisplayMode displayMode = SeriesDisplayMode::Line;
    
    // Y轴区间过滤
    bool filterByRange = false;
    double minValue = 0.0;
    double maxValue = 100.0;
    
    // 散点大小
    int scatterSize = 6;
    
    // 线宽
    int lineWidth = 2;
    
    // Y轴分组（多Y轴模式下使用，相同组号的系列共享Y轴）
    int yAxisGroup = 0;
    
    /**
     * @brief 序列化为JSON
     */
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["displayMode"] = static_cast<int>(displayMode);
        obj["filterByRange"] = filterByRange;
        obj["minValue"] = minValue;
        obj["maxValue"] = maxValue;
        obj["scatterSize"] = scatterSize;
        obj["lineWidth"] = lineWidth;
        obj["yAxisGroup"] = yAxisGroup;
        return obj;
    }
    
    /**
     * @brief 从JSON反序列化
     */
    static SeriesStyle fromJson(const QJsonObject &obj) {
        SeriesStyle style;
        style.displayMode = static_cast<SeriesDisplayMode>(obj["displayMode"].toInt(0));
        style.filterByRange = obj["filterByRange"].toBool(false);
        style.minValue = obj["minValue"].toDouble(0.0);
        style.maxValue = obj["maxValue"].toDouble(100.0);
        style.scatterSize = obj["scatterSize"].toInt(6);
        style.lineWidth = obj["lineWidth"].toInt(2);
        style.yAxisGroup = obj["yAxisGroup"].toInt(0);
        return style;
    }
    
    /**
     * @brief 检查是否为默认样式
     */
    bool isDefault() const {
        return displayMode == SeriesDisplayMode::Line &&
               !filterByRange &&
               scatterSize == 6 &&
               lineWidth == 2 &&
               yAxisGroup == 0;
    }
};

/**
 * @brief 曲线样式设置对话框
 */
class SeriesStyleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SeriesStyleDialog(const QString &seriesName, 
                               const SeriesStyle &currentStyle,
                               QWidget *parent = nullptr);
    
    /**
     * @brief 获取设置后的样式
     */
    SeriesStyle getStyle() const;

private slots:
    void onFilterByRangeChanged(bool checked);
    void onDisplayModeChanged(int index);
    void onResetToDefault();

private:
    void setupUi();
    void applyStyle(const SeriesStyle &style);
    
    QString m_seriesName;
    
    // 显示模式
    QComboBox *m_displayModeCombo;
    
    // 区间过滤
    QCheckBox *m_filterByRangeCheck;
    QDoubleSpinBox *m_minValueSpin;
    QDoubleSpinBox *m_maxValueSpin;
    
    // 散点设置
    QSpinBox *m_scatterSizeSpin;
    
    // 线宽设置
    QSpinBox *m_lineWidthSpin;
    
    // Y轴分组设置
    QSpinBox *m_yAxisGroupSpin;
    
    // 分组框
    QGroupBox *m_rangeGroup;
    QGroupBox *m_scatterGroup;
};

#endif // SERIESSTYLEDIALOG_H
