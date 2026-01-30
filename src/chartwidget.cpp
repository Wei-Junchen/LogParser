#include "chartwidget.h"
#include <QFileInfo>
#include <QPixmap>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_seriesCount(0)
{
    // 创建图表
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::NoAnimation);  // 禁用动画，直接渲染
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    
    // 创建坐标轴
    m_axisX = new QValueAxis();
    m_axisX->setTitleText("X");
    m_axisX->setLabelFormat("%.2f");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    
    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Y");
    m_axisY->setLabelFormat("%.2f");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    
    // 创建图表视图
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    
    // 布局
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_chartView);
    setLayout(m_layout);
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::addSeries(const QString &name, 
                            const QVector<double> &xData, 
                            const QVector<double> &yData,
                            const QColor &color)
{
    if (xData.isEmpty() || yData.isEmpty()) {
        return;
    }
    
    QLineSeries *series = new QLineSeries();
    series->setName(name);
    
    // 设置颜色
    if (color.isValid()) {
        series->setColor(color);
    } else {
        series->setColor(getNextColor());
    }
    
    // 添加数据点
    int count = qMin(xData.size(), yData.size());
    for (int i = 0; i < count; ++i) {
        series->append(xData[i], yData[i]);
    }
    
    // 添加到图表
    m_chart->addSeries(series);
    series->attachAxis(m_axisX);
    series->attachAxis(m_axisY);
    
    m_seriesCount++;
    
    // 自动调整坐标轴
    autoScale();
}

void ChartWidget::clearChart()
{
    m_chart->removeAllSeries();
    m_seriesCount = 0;
    
    // 重置坐标轴
    m_axisX->setRange(0, 1);
    m_axisY->setRange(0, 1);
}

void ChartWidget::setChartTitle(const QString &title)
{
    m_chart->setTitle(title);
}

void ChartWidget::setXAxisLabel(const QString &label)
{
    m_axisX->setTitleText(label);
}

void ChartWidget::setYAxisLabel(const QString &label)
{
    m_axisY->setTitleText(label);
}

void ChartWidget::setLegendVisible(bool visible)
{
    m_chart->legend()->setVisible(visible);
}

void ChartWidget::autoScale()
{
    if (m_chart->series().isEmpty()) {
        return;
    }
    
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    
    for (QAbstractSeries *abstractSeries : m_chart->series()) {
        QLineSeries *series = qobject_cast<QLineSeries*>(abstractSeries);
        if (!series) continue;
        
        for (const QPointF &point : series->points()) {
            minX = qMin(minX, point.x());
            maxX = qMax(maxX, point.x());
            minY = qMin(minY, point.y());
            maxY = qMax(maxY, point.y());
        }
    }
    
    // 添加一些边距
    double marginX = (maxX - minX) * 0.05;
    double marginY = (maxY - minY) * 0.05;
    
    if (marginX == 0) marginX = 1;
    if (marginY == 0) marginY = 1;
    
    m_axisX->setRange(minX - marginX, maxX + marginX);
    m_axisY->setRange(minY - marginY, maxY + marginY);
}

bool ChartWidget::saveAsImage(const QString &filePath)
{
    QPixmap pixmap = m_chartView->grab();
    return pixmap.save(filePath);
}

QColor ChartWidget::getNextColor()
{
    // 预定义的颜色列表
    static const QList<QColor> colors = {
        QColor(31, 119, 180),   // 蓝色
        QColor(255, 127, 14),   // 橙色
        QColor(44, 160, 44),    // 绿色
        QColor(214, 39, 40),    // 红色
        QColor(148, 103, 189),  // 紫色
        QColor(140, 86, 75),    // 棕色
        QColor(227, 119, 194),  // 粉色
        QColor(127, 127, 127),  // 灰色
        QColor(188, 189, 34),   // 黄绿色
        QColor(23, 190, 207)    // 青色
    };
    
    return colors[m_seriesCount % colors.size()];
}
