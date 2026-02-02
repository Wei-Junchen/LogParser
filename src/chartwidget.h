#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QVector>
#include <QString>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QCheckBox>
#include "seriesstyledialog.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using namespace Qt;
QT_USE_NAMESPACE
#else
QT_CHARTS_USE_NAMESPACE
#endif

// 前向声明
class InteractiveChartView;

/**
 * @brief 系列Y轴信息
 */
struct SeriesAxisInfo {
    QString seriesName;
    QValueAxis *yAxis;  // 该系列使用的Y轴
    double yMin;
    double yMax;
};

/**
 * @brief 曲线统计标记信息
 */
struct SeriesMarkerInfo {
    QString seriesName;
    QColor color;
    double yMin;
    double yMax;
    double xMin;
    double xMax;
    bool showYMin = false;
    bool showYMax = false;
    bool showXMin = false;
    bool showXMax = false;
    
    // 标记线（延伸到整个图表区域）
    QLineSeries *yMinLine = nullptr;
    QLineSeries *yMaxLine = nullptr;
    QLineSeries *xMinLine = nullptr;
    QLineSeries *xMaxLine = nullptr;
};

/**
 * @brief 图表组件类
 * 用于显示和管理图表，支持缩放和鼠标悬停显示
 */
class ChartWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();
    
    /**
     * @brief 添加一条数据线
     */
    void addSeries(const QString &name, 
                   const QVector<double> &xData, 
                   const QVector<double> &yData,
                   const QColor &color = QColor(),
                   const SeriesStyle &style = SeriesStyle());
    
    /**
     * @brief 清除所有数据线
     */
    void clearChart();
    
    /**
     * @brief 设置图表标题
     */
    void setChartTitle(const QString &title);
    
    /**
     * @brief 设置X轴标签
     */
    void setXAxisLabel(const QString &label);
    
    /**
     * @brief 设置Y轴标签
     */
    void setYAxisLabel(const QString &label);
    
    /**
     * @brief 设置是否显示图例
     */
    void setLegendVisible(bool visible);
    
    /**
     * @brief 自动调整坐标轴范围
     */
    void autoScale();
    
    /**
     * @brief 保存图表为图片
     */
    bool saveAsImage(const QString &filePath);
    
    /**
     * @brief 获取/设置多Y轴模式
     */
    bool isMultiAxisMode() const { return m_multiAxisMode; }
    void setMultiAxisMode(bool enabled);
    
    /**
     * @brief 获取当前视图范围
     */
    void getViewRange(double &xMin, double &xMax, double &yMin, double &yMax) const;
    
    /**
     * @brief 设置视图范围
     */
    void setViewRange(double xMin, double xMax, double yMin, double yMax);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomReset();
    
    // X轴独立缩放
    void zoomInX();
    void zoomOutX();
    
    // Y轴独立缩放
    void zoomInY();
    void zoomOutY();
    
    /**
     * @brief 显示标记设置对话框
     */
    void showMarkerSettings();

private:
    void setupToolbar();
    QColor getNextColor();
    void updateAxisRanges();
    void updateMarkerLines();
    void clearMarkerLines();

private:
    QChart *m_chart;
    InteractiveChartView *m_chartView;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    QVBoxLayout *m_layout;
    
    // 工具栏
    QWidget *m_toolbar;
    QToolButton *m_zoomInBtn;
    QToolButton *m_zoomOutBtn;
    QToolButton *m_zoomResetBtn;
    QToolButton *m_zoomInXBtn;   // X轴放大
    QToolButton *m_zoomOutXBtn;  // X轴缩小
    QToolButton *m_zoomInYBtn;   // Y轴放大
    QToolButton *m_zoomOutYBtn;  // Y轴缩小
    QToolButton *m_markerBtn;
    QCheckBox *m_multiAxisCheckBox;  // 多Y轴模式开关
    
    int m_seriesCount;
    
    // 原始坐标轴范围
    double m_originalXMin, m_originalXMax;
    double m_originalYMin, m_originalYMax;
    
    // 多Y轴模式
    bool m_multiAxisMode;
    QList<SeriesAxisInfo> m_seriesAxisInfos;  // 每个系列的Y轴信息
    QList<QValueAxis*> m_extraYAxes;  // 额外的Y轴列表
    QMap<int, QValueAxis*> m_yAxisGroups;  // Y轴组映射（组号 -> Y轴）
    
    // 曲线标记信息
    QList<SeriesMarkerInfo> m_markerInfos;
};

/**
 * @brief 自定义ChartView，支持鼠标悬停显示垂直线和数值，以及拖拽平移
 */
class InteractiveChartView : public QChartView
{
    Q_OBJECT
    
public:
    explicit InteractiveChartView(QChart *chart, QWidget *parent = nullptr);
    ~InteractiveChartView();
    
    void setAxes(QValueAxis *axisX, QValueAxis *axisY);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateCrosshair(const QPoint &pos);
    void hideCrosshair();
    QString buildTooltipText(double xValue, double yValue);
    double interpolateY(QLineSeries *series, double xValue);
    double interpolateYScatter(QScatterSeries *series, double xValue);

private:
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    
    // 拖拽相关
    bool m_isDragging;
    QPoint m_lastMousePos;
    
    // 垂直虚线
    QGraphicsLineItem *m_verticalLine;
    // 水平虚线
    QGraphicsLineItem *m_horizontalLine;
    // 提示框背景
    QGraphicsRectItem *m_tooltipBg;
    // 提示文字
    QGraphicsTextItem *m_tooltipText;
};

#endif // CHARTWIDGET_H
