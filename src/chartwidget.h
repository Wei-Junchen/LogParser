#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QVector>
#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using namespace Qt;
#endif

QT_CHARTS_USE_NAMESPACE

/**
 * @brief 图表组件类
 * 用于显示和管理图表
 */
class ChartWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();
    
    /**
     * @brief 添加一条数据线
     * @param name 数据线名称
     * @param xData X轴数据
     * @param yData Y轴数据
     * @param color 线条颜色（可选）
     */
    void addSeries(const QString &name, 
                   const QVector<double> &xData, 
                   const QVector<double> &yData,
                   const QColor &color = QColor());
    
    /**
     * @brief 清除所有数据线
     */
    void clearChart();
    
    /**
     * @brief 设置图表标题
     * @param title 标题
     */
    void setChartTitle(const QString &title);
    
    /**
     * @brief 设置X轴标签
     * @param label X轴标签
     */
    void setXAxisLabel(const QString &label);
    
    /**
     * @brief 设置Y轴标签
     * @param label Y轴标签
     */
    void setYAxisLabel(const QString &label);
    
    /**
     * @brief 设置是否显示图例
     * @param visible 是否显示
     */
    void setLegendVisible(bool visible);
    
    /**
     * @brief 自动调整坐标轴范围
     */
    void autoScale();
    
    /**
     * @brief 保存图表为图片
     * @param filePath 保存路径
     * @return 是否成功
     */
    bool saveAsImage(const QString &filePath);

private:
    QChart *m_chart;
    QChartView *m_chartView;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QVBoxLayout *m_layout;
    
    int m_seriesCount;
    
    /**
     * @brief 获取下一个颜色
     * @return 颜色
     */
    QColor getNextColor();
};

#endif // CHARTWIDGET_H
