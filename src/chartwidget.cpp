#include "chartwidget.h"
#include <QHBoxLayout>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QGridLayout>
#include <QFontMetrics>
#include <cmath>
#include <limits>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QT_USE_NAMESPACE
#else
QT_CHARTS_USE_NAMESPACE
#endif

namespace {
bool supportsEmojiGlyph(const QFont &font, char32_t codepoint)
{
    QFontMetrics fm(font);
    return fm.inFontUcs4(static_cast<uint>(codepoint));
}

QFont emojiPreferredFont(const QFont &base)
{
    QFont f(base);
    QStringList families;
    families << "Noto Color Emoji" << base.family();
    f.setFamilies(families);
    return f;
}

bool hasVisibleMarkerLines(const QList<SeriesMarkerInfo> &infos)
{
    for (const SeriesMarkerInfo &info : infos) {
        if (info.showYMin || info.showYMax || info.showXMin || info.showXMax) {
            return true;
        }
    }
    return false;
}
}

// ==================== ChartWidget ====================

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_seriesCount(0)
    , m_originalXMin(0), m_originalXMax(1)
    , m_originalYMin(0), m_originalYMax(1)
    , m_multiAxisMode(false)
    , m_showZeroLine(false)
    , m_zeroLine(nullptr)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(2);
    
    // 创建工具栏
    setupToolbar();
    
    // 创建图表
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->setMargins(QMargins(5, 5, 5, 5));
    
    // 创建坐标轴
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    // 轴范围变化时刷新标记线，避免缩放/平移后标记线位置过期
    connect(m_axisX, &QValueAxis::rangeChanged, this, [this](qreal, qreal) {
        if (m_showZeroLine || hasVisibleMarkerLines(m_markerInfos)) {
            updateMarkerLines();
        }
    });
    connect(m_axisY, &QValueAxis::rangeChanged, this, [this](qreal, qreal) {
        if (m_showZeroLine || hasVisibleMarkerLines(m_markerInfos)) {
            updateMarkerLines();
        }
    });
    
    // 创建交互式图表视图
    m_chartView = new InteractiveChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMouseTracking(true);
    m_chartView->setAxes(m_axisX, m_axisY);
    
    m_layout->addWidget(m_chartView);
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::setupToolbar()
{
    m_toolbar = new QWidget(this);
    QHBoxLayout *toolLayout = new QHBoxLayout(m_toolbar);
    toolLayout->setContentsMargins(5, 2, 5, 2);
    toolLayout->setSpacing(5);
    
    // 放大按钮
    m_zoomInBtn = new QToolButton();
    m_zoomInBtn->setFont(emojiPreferredFont(m_zoomInBtn->font()));
    m_zoomInBtn->setText(supportsEmojiGlyph(m_zoomInBtn->font(), U'🔍') ? "🔍+" : "放+");
    m_zoomInBtn->setToolTip("整体放大 (X和Y轴)");
    m_zoomInBtn->setFixedSize(32, 28);
    connect(m_zoomInBtn, &QToolButton::clicked, this, &ChartWidget::zoomIn);
    
    // 缩小按钮
    m_zoomOutBtn = new QToolButton();
    m_zoomOutBtn->setFont(emojiPreferredFont(m_zoomOutBtn->font()));
    m_zoomOutBtn->setText(supportsEmojiGlyph(m_zoomOutBtn->font(), U'🔍') ? "🔍-" : "缩-");
    m_zoomOutBtn->setToolTip("整体缩小 (X和Y轴)");
    m_zoomOutBtn->setFixedSize(32, 28);
    connect(m_zoomOutBtn, &QToolButton::clicked, this, &ChartWidget::zoomOut);
    
    // X轴放大按钮
    m_zoomInXBtn = new QToolButton();
    m_zoomInXBtn->setText("X+");
    m_zoomInXBtn->setToolTip("X轴放大");
    m_zoomInXBtn->setFixedSize(32, 28);
    connect(m_zoomInXBtn, &QToolButton::clicked, this, &ChartWidget::zoomInX);
    
    // X轴缩小按钮
    m_zoomOutXBtn = new QToolButton();
    m_zoomOutXBtn->setText("X-");
    m_zoomOutXBtn->setToolTip("X轴缩小");
    m_zoomOutXBtn->setFixedSize(32, 28);
    connect(m_zoomOutXBtn, &QToolButton::clicked, this, &ChartWidget::zoomOutX);
    
    // Y轴放大按钮
    m_zoomInYBtn = new QToolButton();
    m_zoomInYBtn->setText("Y+");
    m_zoomInYBtn->setToolTip("Y轴放大");
    m_zoomInYBtn->setFixedSize(32, 28);
    connect(m_zoomInYBtn, &QToolButton::clicked, this, &ChartWidget::zoomInY);
    
    // Y轴缩小按钮
    m_zoomOutYBtn = new QToolButton();
    m_zoomOutYBtn->setText("Y-");
    m_zoomOutYBtn->setToolTip("Y轴缩小");
    m_zoomOutYBtn->setFixedSize(32, 28);
    connect(m_zoomOutYBtn, &QToolButton::clicked, this, &ChartWidget::zoomOutY);
    
    // 重置按钮
    m_zoomResetBtn = new QToolButton();
    m_zoomResetBtn->setText("↺");
    m_zoomResetBtn->setToolTip("重置视图");
    m_zoomResetBtn->setFixedSize(32, 28);
    connect(m_zoomResetBtn, &QToolButton::clicked, this, &ChartWidget::zoomReset);
    
    toolLayout->addWidget(new QLabel("缩放:"));
    toolLayout->addWidget(m_zoomInBtn);
    toolLayout->addWidget(m_zoomOutBtn);
    toolLayout->addWidget(new QLabel("|"));
    toolLayout->addWidget(m_zoomInXBtn);
    toolLayout->addWidget(m_zoomOutXBtn);
    toolLayout->addWidget(new QLabel("|"));
    toolLayout->addWidget(m_zoomInYBtn);
    toolLayout->addWidget(m_zoomOutYBtn);
    toolLayout->addWidget(new QLabel("|"));
    toolLayout->addWidget(m_zoomResetBtn);
    
    // 分隔符
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    toolLayout->addWidget(separator);
    
    // 多Y轴模式复选框
    m_multiAxisCheckBox = new QCheckBox("多-Y轴模式");
    m_multiAxisCheckBox->setToolTip("为每个数据系列使用独立的Y轴，\n适用于数值范围差异很大的数据");
    m_multiAxisCheckBox->setChecked(false);
    connect(m_multiAxisCheckBox, &QCheckBox::toggled, this, [this](bool checked) {
        m_multiAxisMode = checked;
        // 重新绘制图表
        // 保存当前数据
        QList<QPair<QString, QPair<QList<QPointF>, QPair<QColor, SeriesStyle>>>> seriesData;
        for (QAbstractSeries *abstractSeries : m_chart->series()) {
            QLineSeries *lineSeries = qobject_cast<QLineSeries*>(abstractSeries);
            QScatterSeries *scatterSeries = qobject_cast<QScatterSeries*>(abstractSeries);
            
            if (lineSeries && !lineSeries->name().endsWith(" (点)")) {
                QList<QPointF> points = lineSeries->points();
                QColor color = lineSeries->color();
                SeriesStyle style;
                style.displayMode = SeriesDisplayMode::Line;
                style.lineWidth = lineSeries->pen().width();
                seriesData.append(qMakePair(lineSeries->name(), qMakePair(points, qMakePair(color, style))));
            } else if (scatterSeries && !scatterSeries->name().endsWith(" (点)")) {
                QList<QPointF> points = scatterSeries->points();
                QColor color = scatterSeries->color();
                SeriesStyle style;
                style.displayMode = SeriesDisplayMode::Scatter;
                style.scatterSize = scatterSeries->markerSize();
                seriesData.append(qMakePair(scatterSeries->name(), qMakePair(points, qMakePair(color, style))));
            }
        }
        
        // 清除并重新添加
        clearChart();
        for (const auto &data : seriesData) {
            QVector<double> xData, yData;
            for (const QPointF &p : data.second.first) {
                xData.append(p.x());
                yData.append(p.y());
            }
            addSeries(data.first, xData, yData, data.second.second.first, data.second.second.second);
        }
    });
    toolLayout->addWidget(m_multiAxisCheckBox);

    // Y=0 基准线复选框
    m_zeroLineCheckBox = new QCheckBox("Y=0基准线");
    m_zeroLineCheckBox->setToolTip("显示/隐藏 y=0 水平基准线");
    m_zeroLineCheckBox->setChecked(m_showZeroLine);
    connect(m_zeroLineCheckBox, &QCheckBox::toggled, this, &ChartWidget::setZeroLineVisible);
    toolLayout->addWidget(m_zeroLineCheckBox);
    
    // 分隔符
    QFrame *separator2 = new QFrame();
    separator2->setFrameShape(QFrame::VLine);
    separator2->setFrameShadow(QFrame::Sunken);
    toolLayout->addWidget(separator2);
    
    // 标记按钮
    m_markerBtn = new QToolButton();
    m_markerBtn->setFont(emojiPreferredFont(m_markerBtn->font()));
    m_markerBtn->setText(supportsEmojiGlyph(m_markerBtn->font(), U'📏') ? "📏" : "标");
    m_markerBtn->setToolTip("显示/隐藏曲线标记线 (Y最大/最小值, X起止值)");
    m_markerBtn->setFixedSize(32, 28);
    connect(m_markerBtn, &QToolButton::clicked, this, &ChartWidget::showMarkerSettings);
    toolLayout->addWidget(new QLabel("标记:"));
    toolLayout->addWidget(m_markerBtn);
    
    toolLayout->addStretch();
    
    m_layout->addWidget(m_toolbar);
}

void ChartWidget::addSeries(const QString &name, 
                            const QVector<double> &xData, 
                            const QVector<double> &yData,
                            const QColor &color,
                            const SeriesStyle &style)
{
    if (xData.isEmpty() || yData.isEmpty()) {
        return;
    }
    
    int count = qMin(xData.size(), yData.size());
    
    // 根据样式过滤和准备数据
    QVector<double> filteredX, filteredY;
    for (int i = 0; i < count; ++i) {
        double y = yData[i];
        
        // 区间过滤
        if (style.filterByRange) {
            if (y < style.minValue || y > style.maxValue) {
                continue;  // 跳过超出区间的点
            }
        }
        
        filteredX.append(xData[i]);
        filteredY.append(y);
    }
    
    if (filteredX.isEmpty()) {
        return;  // 过滤后没有数据
    }
    
    // 确定使用的颜色
    QColor seriesColor = color.isValid() ? color : getNextColor();
    
    // 确定使用的Y轴
    QValueAxis *yAxisToUse = m_axisY;
    if (m_multiAxisMode) {
        // 多Y轴模式：根据Y轴分组决定是否共享Y轴
        int groupId = style.yAxisGroup;
        
        if (groupId == 0) {
            // 组号为0表示独立Y轴，为每个系列创建新的Y轴
            yAxisToUse = new QValueAxis();
            yAxisToUse->setTitleText(name);
            yAxisToUse->setLinePenColor(seriesColor);
            yAxisToUse->setLabelsColor(seriesColor);
            
            // 交替使用左右两侧
            Qt::Alignment alignment = (m_seriesCount % 2 == 0) ? Qt::AlignLeft : Qt::AlignRight;
            m_chart->addAxis(yAxisToUse, alignment);
            
            m_extraYAxes.append(yAxisToUse);
        } else {
            // 非0组号：查找是否已有相同组号的Y轴
            if (m_yAxisGroups.contains(groupId)) {
                // 使用已存在的组Y轴
                yAxisToUse = m_yAxisGroups[groupId];
            } else {
                // 创建新的组Y轴
                yAxisToUse = new QValueAxis();
                yAxisToUse->setTitleText(QString("[组%1] %2").arg(groupId).arg(name));
                yAxisToUse->setLinePenColor(seriesColor);
                yAxisToUse->setLabelsColor(seriesColor);
                
                // 交替使用左右两侧
                Qt::Alignment alignment = (m_yAxisGroups.size() % 2 == 0) ? Qt::AlignLeft : Qt::AlignRight;
                m_chart->addAxis(yAxisToUse, alignment);
                
                m_extraYAxes.append(yAxisToUse);
                m_yAxisGroups[groupId] = yAxisToUse;
            }
        }
    }
    
    // 根据显示模式创建不同类型的Series
    switch (style.displayMode) {
        case SeriesDisplayMode::Line: {
            // 连线模式
            QLineSeries *series = new QLineSeries();
            series->setName(name);
            
            for (int i = 0; i < filteredX.size(); ++i) {
                series->append(filteredX[i], filteredY[i]);
            }
            
            series->setColor(seriesColor);
            
            QPen pen = series->pen();
            pen.setWidth(style.lineWidth);
            series->setPen(pen);
            
            m_chart->addSeries(series);
            series->attachAxis(m_axisX);
            series->attachAxis(yAxisToUse);
            break;
        }
        
        case SeriesDisplayMode::Scatter: {
            // 散点模式
            QScatterSeries *series = new QScatterSeries();
            series->setName(name);
            
            for (int i = 0; i < filteredX.size(); ++i) {
                series->append(filteredX[i], filteredY[i]);
            }
            
            series->setColor(seriesColor);
            series->setMarkerSize(style.scatterSize);
            series->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            series->setBorderColor(seriesColor);
            
            m_chart->addSeries(series);
            series->attachAxis(m_axisX);
            series->attachAxis(yAxisToUse);
            break;
        }
        
        case SeriesDisplayMode::LineAndScatter: {
            // 连线+散点模式
            // 先添加连线
            QLineSeries *lineSeries = new QLineSeries();
            lineSeries->setName(name);
            
            for (int i = 0; i < filteredX.size(); ++i) {
                lineSeries->append(filteredX[i], filteredY[i]);
            }
            
            lineSeries->setColor(seriesColor);
            
            QPen pen = lineSeries->pen();
            pen.setWidth(style.lineWidth);
            lineSeries->setPen(pen);
            
            m_chart->addSeries(lineSeries);
            lineSeries->attachAxis(m_axisX);
            lineSeries->attachAxis(yAxisToUse);
            
            // 再添加散点（不显示在图例中）
            QScatterSeries *scatterSeries = new QScatterSeries();
            scatterSeries->setName(name + " (点)");
            
            for (int i = 0; i < filteredX.size(); ++i) {
                scatterSeries->append(filteredX[i], filteredY[i]);
            }
            
            scatterSeries->setColor(seriesColor);
            scatterSeries->setMarkerSize(style.scatterSize);
            scatterSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
            scatterSeries->setBorderColor(seriesColor);
            
            m_chart->addSeries(scatterSeries);
            scatterSeries->attachAxis(m_axisX);
            scatterSeries->attachAxis(yAxisToUse);
            
            // 隐藏散点的图例
            m_chart->legend()->markers(scatterSeries).first()->setVisible(false);
            break;
        }
    }
    
    // 收集曲线统计信息
    SeriesMarkerInfo markerInfo;
    markerInfo.seriesName = name;
    markerInfo.color = seriesColor;
    markerInfo.yMin = std::numeric_limits<double>::max();
    markerInfo.yMax = std::numeric_limits<double>::lowest();
    markerInfo.xMin = std::numeric_limits<double>::max();
    markerInfo.xMax = std::numeric_limits<double>::lowest();
    
    for (int i = 0; i < filteredX.size(); ++i) {
        markerInfo.xMin = qMin(markerInfo.xMin, filteredX[i]);
        markerInfo.xMax = qMax(markerInfo.xMax, filteredX[i]);
        markerInfo.yMin = qMin(markerInfo.yMin, filteredY[i]);
        markerInfo.yMax = qMax(markerInfo.yMax, filteredY[i]);
    }
    
    m_markerInfos.append(markerInfo);
    
    // 保存系列轴信息
    if (m_multiAxisMode) {
        SeriesAxisInfo axisInfo;
        axisInfo.seriesName = name;
        axisInfo.yAxis = yAxisToUse;
        axisInfo.yMin = markerInfo.yMin;
        axisInfo.yMax = markerInfo.yMax;
        m_seriesAxisInfos.append(axisInfo);
    }
    
    m_seriesCount++;
    
    // 更新坐标轴范围
    updateAxisRanges();
}

void ChartWidget::updateAxisRanges()
{
    if (m_chart->series().isEmpty()) {
        return;
    }
    
    if (m_multiAxisMode) {
        // 多Y轴模式：为每个Y轴单独设置范围
        double xMin = std::numeric_limits<double>::max();
        double xMax = std::numeric_limits<double>::lowest();
        
        // 计算X轴范围
        for (QAbstractSeries *abstractSeries : m_chart->series()) {
            QLineSeries *lineSeries = qobject_cast<QLineSeries*>(abstractSeries);
            QScatterSeries *scatterSeries = qobject_cast<QScatterSeries*>(abstractSeries);
            
            if (lineSeries) {
                for (const QPointF &point : lineSeries->points()) {
                    xMin = qMin(xMin, point.x());
                    xMax = qMax(xMax, point.x());
                }
            } else if (scatterSeries) {
                for (const QPointF &point : scatterSeries->points()) {
                    xMin = qMin(xMin, point.x());
                    xMax = qMax(xMax, point.x());
                }
            }
        }
        
        double xMargin = (xMax - xMin) * 0.02;
        if (xMargin == 0) xMargin = 1;
        m_axisX->setRange(xMin - xMargin, xMax + xMargin);
        m_originalXMin = xMin - xMargin;
        m_originalXMax = xMax + xMargin;
        
        // 为每个独立的Y轴设置范围
        for (const SeriesAxisInfo &axisInfo : m_seriesAxisInfos) {
            double yMin = axisInfo.yMin;
            double yMax = axisInfo.yMax;
            double yMargin = (yMax - yMin) * 0.05;
            if (yMargin == 0) yMargin = qAbs(yMin) * 0.1;
            if (yMargin == 0) yMargin = 1;
            
            axisInfo.yAxis->setRange(yMin - yMargin, yMax + yMargin);
        }
    } else {
        // 单Y轴模式：所有系列使用相同的Y轴范围
        double xMin = std::numeric_limits<double>::max();
        double xMax = std::numeric_limits<double>::lowest();
        double yMin = std::numeric_limits<double>::max();
        double yMax = std::numeric_limits<double>::lowest();
        
        for (QAbstractSeries *abstractSeries : m_chart->series()) {
            QLineSeries *lineSeries = qobject_cast<QLineSeries*>(abstractSeries);
            QScatterSeries *scatterSeries = qobject_cast<QScatterSeries*>(abstractSeries);
            
            if (lineSeries) {
                for (const QPointF &point : lineSeries->points()) {
                    xMin = qMin(xMin, point.x());
                    xMax = qMax(xMax, point.x());
                    yMin = qMin(yMin, point.y());
                    yMax = qMax(yMax, point.y());
                }
            } else if (scatterSeries) {
                for (const QPointF &point : scatterSeries->points()) {
                    xMin = qMin(xMin, point.x());
                    xMax = qMax(xMax, point.x());
                    yMin = qMin(yMin, point.y());
                    yMax = qMax(yMax, point.y());
                }
            }
        }
        
        // 添加一点边距
        double xMargin = (xMax - xMin) * 0.02;
        double yMargin = (yMax - yMin) * 0.05;
        
        if (xMargin == 0) xMargin = 1;
        if (yMargin == 0) yMargin = 1;
        
        m_axisX->setRange(xMin - xMargin, xMax + xMargin);
        m_axisY->setRange(yMin - yMargin, yMax + yMargin);
        
        // 保存原始范围用于重置
        m_originalXMin = xMin - xMargin;
        m_originalXMax = xMax + xMargin;
        m_originalYMin = yMin - yMargin;
        m_originalYMax = yMax + yMargin;
    }
}

void ChartWidget::clearChart()
{
    clearMarkerLines();
    m_markerInfos.clear();
    m_chart->removeAllSeries();
    
    // 清除额外的Y轴
    for (QValueAxis *axis : m_extraYAxes) {
        m_chart->removeAxis(axis);
        delete axis;
    }
    m_extraYAxes.clear();
    m_seriesAxisInfos.clear();
    m_yAxisGroups.clear();  // 清除Y轴组映射
    
    m_seriesCount = 0;
    
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
    updateAxisRanges();
}

bool ChartWidget::saveAsImage(const QString &filePath)
{
    QPixmap pixmap = m_chartView->grab();
    return pixmap.save(filePath);
}

void ChartWidget::setMultiAxisMode(bool enabled)
{
    if (m_multiAxisMode == enabled) {
        return;
    }
    
    // 通过触发复选框来切换模式（这样可以重用现有的切换逻辑）
    m_multiAxisCheckBox->setChecked(enabled);
}

void ChartWidget::setZeroLineVisible(bool visible)
{
    m_showZeroLine = visible;
    if (m_zeroLineCheckBox && m_zeroLineCheckBox->isChecked() != visible) {
        m_zeroLineCheckBox->blockSignals(true);
        m_zeroLineCheckBox->setChecked(visible);
        m_zeroLineCheckBox->blockSignals(false);
    }
    updateMarkerLines();
}

void ChartWidget::getViewRange(double &xMin, double &xMax, double &yMin, double &yMax) const
{
    xMin = m_axisX->min();
    xMax = m_axisX->max();
    yMin = m_axisY->min();
    yMax = m_axisY->max();
}

void ChartWidget::setViewRange(double xMin, double xMax, double yMin, double yMax)
{
    m_axisX->setRange(xMin, xMax);
    if (!m_multiAxisMode) {
        m_axisY->setRange(yMin, yMax);
    }
}

void ChartWidget::zoomIn()
{
    m_chart->zoomIn();
}

void ChartWidget::zoomOut()
{
    m_chart->zoomOut();
}

void ChartWidget::zoomInX()
{
    double xMin = m_axisX->min();
    double xMax = m_axisX->max();
    double xCenter = (xMin + xMax) / 2.0;
    double xRange = (xMax - xMin) * 0.8 / 2.0;
    
    m_axisX->setRange(xCenter - xRange, xCenter + xRange);
}

void ChartWidget::zoomOutX()
{
    double xMin = m_axisX->min();
    double xMax = m_axisX->max();
    double xCenter = (xMin + xMax) / 2.0;
    double xRange = (xMax - xMin) * 1.25 / 2.0;
    
    m_axisX->setRange(xCenter - xRange, xCenter + xRange);
}

void ChartWidget::zoomInY()
{
    if (m_multiAxisMode) {
        // 多Y轴模式：缩放所有Y轴
        for (const SeriesAxisInfo &axisInfo : m_seriesAxisInfos) {
            double yMin = axisInfo.yAxis->min();
            double yMax = axisInfo.yAxis->max();
            double yCenter = (yMin + yMax) / 2.0;
            double yRange = (yMax - yMin) * 0.8 / 2.0;
            
            axisInfo.yAxis->setRange(yCenter - yRange, yCenter + yRange);
        }
    } else {
        double yMin = m_axisY->min();
        double yMax = m_axisY->max();
        double yCenter = (yMin + yMax) / 2.0;
        double yRange = (yMax - yMin) * 0.8 / 2.0;
        
        m_axisY->setRange(yCenter - yRange, yCenter + yRange);
    }
}

void ChartWidget::zoomOutY()
{
    if (m_multiAxisMode) {
        // 多Y轴模式：缩放所有Y轴
        for (const SeriesAxisInfo &axisInfo : m_seriesAxisInfos) {
            double yMin = axisInfo.yAxis->min();
            double yMax = axisInfo.yAxis->max();
            double yCenter = (yMin + yMax) / 2.0;
            double yRange = (yMax - yMin) * 1.25 / 2.0;
            
            axisInfo.yAxis->setRange(yCenter - yRange, yCenter + yRange);
        }
    } else {
        double yMin = m_axisY->min();
        double yMax = m_axisY->max();
        double yCenter = (yMin + yMax) / 2.0;
        double yRange = (yMax - yMin) * 1.25 / 2.0;
        
        m_axisY->setRange(yCenter - yRange, yCenter + yRange);
    }
}

void ChartWidget::zoomReset()
{
    m_axisX->setRange(m_originalXMin, m_originalXMax);
    
    if (m_multiAxisMode) {
        // 多Y轴模式：重置每个Y轴
        for (const SeriesAxisInfo &axisInfo : m_seriesAxisInfos) {
            double yMin = axisInfo.yMin;
            double yMax = axisInfo.yMax;
            double yMargin = (yMax - yMin) * 0.05;
            if (yMargin == 0) yMargin = qAbs(yMin) * 0.1;
            if (yMargin == 0) yMargin = 1;
            
            axisInfo.yAxis->setRange(yMin - yMargin, yMax + yMargin);
        }
    } else {
        m_axisY->setRange(m_originalYMin, m_originalYMax);
    }
}

QColor ChartWidget::getNextColor()
{
    static QList<QColor> colors = {
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

void ChartWidget::showMarkerSettings()
{
    if (m_markerInfos.isEmpty()) {
        return;
    }
    
    QDialog dialog(this);
    dialog.setWindowTitle("曲线标记设置");
    dialog.setMinimumWidth(500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    // 说明标签
    QLabel *infoLabel = new QLabel("勾选要显示的标记线（延伸至整个图表区域）：");
    mainLayout->addWidget(infoLabel);
    
    // 滚动区域
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget *scrollWidget = new QWidget();
    QGridLayout *grid = new QGridLayout(scrollWidget);
    
    // 表头
    grid->addWidget(new QLabel("<b>曲线</b>"), 0, 0);
    grid->addWidget(new QLabel("<b>Y最小值</b>"), 0, 1);
    grid->addWidget(new QLabel("<b>Y最大值</b>"), 0, 2);
    grid->addWidget(new QLabel("<b>X起始</b>"), 0, 3);
    grid->addWidget(new QLabel("<b>X终止</b>"), 0, 4);
    
    // 存储复选框
    QList<QCheckBox*> yMinChecks, yMaxChecks, xMinChecks, xMaxChecks;
    
    for (int i = 0; i < m_markerInfos.size(); ++i) {
        const SeriesMarkerInfo &info = m_markerInfos[i];
        
        // 曲线名称（带颜色指示）
        QLabel *nameLabel = new QLabel(QString("<font color='%1'>●</font> %2")
                                       .arg(info.color.name())
                                       .arg(info.seriesName));
        grid->addWidget(nameLabel, i + 1, 0);
        
        // Y最小值
        QCheckBox *yMinCheck = new QCheckBox(QString::number(info.yMin, 'f', 4));
        yMinCheck->setChecked(info.showYMin);
        grid->addWidget(yMinCheck, i + 1, 1);
        yMinChecks.append(yMinCheck);
        
        // Y最大值
        QCheckBox *yMaxCheck = new QCheckBox(QString::number(info.yMax, 'f', 4));
        yMaxCheck->setChecked(info.showYMax);
        grid->addWidget(yMaxCheck, i + 1, 2);
        yMaxChecks.append(yMaxCheck);
        
        // X起始值
        QCheckBox *xMinCheck = new QCheckBox(QString::number(info.xMin, 'f', 4));
        xMinCheck->setChecked(info.showXMin);
        grid->addWidget(xMinCheck, i + 1, 3);
        xMinChecks.append(xMinCheck);
        
        // X终止值
        QCheckBox *xMaxCheck = new QCheckBox(QString::number(info.xMax, 'f', 4));
        xMaxCheck->setChecked(info.showXMax);
        grid->addWidget(xMaxCheck, i + 1, 4);
        xMaxChecks.append(xMaxCheck);
    }
    
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea);
    
    // 按钮
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 更新标记设置
        for (int i = 0; i < m_markerInfos.size(); ++i) {
            m_markerInfos[i].showYMin = yMinChecks[i]->isChecked();
            m_markerInfos[i].showYMax = yMaxChecks[i]->isChecked();
            m_markerInfos[i].showXMin = xMinChecks[i]->isChecked();
            m_markerInfos[i].showXMax = xMaxChecks[i]->isChecked();
        }
        
        // 重新绘制标记线
        updateMarkerLines();
    }
}

void ChartWidget::clearMarkerLines()
{
    if (m_zeroLine) {
        m_chart->removeSeries(m_zeroLine);
        m_zeroLine = nullptr;
    }

    for (SeriesMarkerInfo &info : m_markerInfos) {
        if (info.yMinLine) {
            m_chart->removeSeries(info.yMinLine);
            info.yMinLine = nullptr;
        }
        if (info.yMaxLine) {
            m_chart->removeSeries(info.yMaxLine);
            info.yMaxLine = nullptr;
        }
        if (info.xMinLine) {
            m_chart->removeSeries(info.xMinLine);
            info.xMinLine = nullptr;
        }
        if (info.xMaxLine) {
            m_chart->removeSeries(info.xMaxLine);
            info.xMaxLine = nullptr;
        }
    }
}

void ChartWidget::updateMarkerLines()
{
    // 先清除现有标记线
    clearMarkerLines();

    if (m_chart->series().isEmpty()) {
        return;
    }
    
    // 获取当前坐标轴范围用于绘制延伸线
    double axisXMin = m_axisX->min();
    double axisXMax = m_axisX->max();
    double axisYMin = m_axisY->min();
    double axisYMax = m_axisY->max();

    // Y=0 基准线
    if (m_showZeroLine) {
        m_zeroLine = new QLineSeries();
        m_zeroLine->setName("Y=0 基准线");
        m_zeroLine->append(axisXMin, 0.0);
        m_zeroLine->append(axisXMax, 0.0);

        QPen zeroPen(QColor(120, 120, 120));
        zeroPen.setStyle(Qt::DashLine);
        zeroPen.setWidth(2);
        m_zeroLine->setPen(zeroPen);

        m_chart->addSeries(m_zeroLine);
        m_zeroLine->attachAxis(m_axisX);
        m_zeroLine->attachAxis(m_axisY);

        auto markers = m_chart->legend()->markers(m_zeroLine);
        if (!markers.isEmpty()) {
            markers.first()->setVisible(false);
        }
    }
    
    for (SeriesMarkerInfo &info : m_markerInfos) {
        QColor lighterColor = info.color.lighter(120);
        
        // Y最小值水平线
        if (info.showYMin) {
            info.yMinLine = new QLineSeries();
            info.yMinLine->setName(QString("%1 Y_min").arg(info.seriesName));
            info.yMinLine->append(axisXMin, info.yMin);
            info.yMinLine->append(axisXMax, info.yMin);
            
            QPen pen(lighterColor);
            pen.setStyle(Qt::DashLine);
            pen.setWidth(2);
            info.yMinLine->setPen(pen);
            
            m_chart->addSeries(info.yMinLine);
            info.yMinLine->attachAxis(m_axisX);
            info.yMinLine->attachAxis(m_axisY);
            
            // 隐藏图例
            auto markers = m_chart->legend()->markers(info.yMinLine);
            if (!markers.isEmpty()) {
                markers.first()->setVisible(false);
            }
        }
        
        // Y最大值水平线
        if (info.showYMax) {
            info.yMaxLine = new QLineSeries();
            info.yMaxLine->setName(QString("%1 Y_max").arg(info.seriesName));
            info.yMaxLine->append(axisXMin, info.yMax);
            info.yMaxLine->append(axisXMax, info.yMax);
            
            QPen pen(lighterColor);
            pen.setStyle(Qt::DashDotLine);
            pen.setWidth(2);
            info.yMaxLine->setPen(pen);
            
            m_chart->addSeries(info.yMaxLine);
            info.yMaxLine->attachAxis(m_axisX);
            info.yMaxLine->attachAxis(m_axisY);
            
            // 隐藏图例
            auto markers = m_chart->legend()->markers(info.yMaxLine);
            if (!markers.isEmpty()) {
                markers.first()->setVisible(false);
            }
        }
        
        // X起始垂直线
        if (info.showXMin) {
            info.xMinLine = new QLineSeries();
            info.xMinLine->setName(QString("%1 X_start").arg(info.seriesName));
            info.xMinLine->append(info.xMin, axisYMin);
            info.xMinLine->append(info.xMin, axisYMax);
            
            QPen pen(lighterColor);
            pen.setStyle(Qt::DotLine);
            pen.setWidth(2);
            info.xMinLine->setPen(pen);
            
            m_chart->addSeries(info.xMinLine);
            info.xMinLine->attachAxis(m_axisX);
            info.xMinLine->attachAxis(m_axisY);
            
            // 隐藏图例
            auto markers = m_chart->legend()->markers(info.xMinLine);
            if (!markers.isEmpty()) {
                markers.first()->setVisible(false);
            }
        }
        
        // X终止垂直线
        if (info.showXMax) {
            info.xMaxLine = new QLineSeries();
            info.xMaxLine->setName(QString("%1 X_end").arg(info.seriesName));
            info.xMaxLine->append(info.xMax, axisYMin);
            info.xMaxLine->append(info.xMax, axisYMax);
            
            QPen pen(lighterColor);
            pen.setStyle(Qt::DashDotDotLine);
            pen.setWidth(2);
            info.xMaxLine->setPen(pen);
            
            m_chart->addSeries(info.xMaxLine);
            info.xMaxLine->attachAxis(m_axisX);
            info.xMaxLine->attachAxis(m_axisY);
            
            // 隐藏图例
            auto markers = m_chart->legend()->markers(info.xMaxLine);
            if (!markers.isEmpty()) {
                markers.first()->setVisible(false);
            }
        }
    }
}

// ==================== InteractiveChartView ====================

InteractiveChartView::InteractiveChartView(QChart *chart, QWidget *parent)
    : QChartView(chart, parent)
    , m_axisX(nullptr)
    , m_axisY(nullptr)
    , m_isDragging(false)
    , m_lastMousePos()
    , m_verticalLine(nullptr)
    , m_horizontalLine(nullptr)
    , m_tooltipBg(nullptr)
    , m_tooltipText(nullptr)
{
    setMouseTracking(true);
    
    // 创建垂直虚线
    QPen dashedPen(Qt::gray);
    dashedPen.setStyle(Qt::DashLine);
    dashedPen.setWidth(1);
    m_verticalLine = new QGraphicsLineItem();
    m_verticalLine->setPen(dashedPen);
    m_verticalLine->setVisible(false);
    scene()->addItem(m_verticalLine);
    
    // 创建水平虚线
    m_horizontalLine = new QGraphicsLineItem();
    m_horizontalLine->setPen(dashedPen);
    m_horizontalLine->setVisible(false);
    scene()->addItem(m_horizontalLine);
    
    // 创建提示框背景
    m_tooltipBg = new QGraphicsRectItem();
    m_tooltipBg->setBrush(QBrush(QColor(255, 255, 225, 230)));
    m_tooltipBg->setPen(QPen(Qt::gray));
    m_tooltipBg->setVisible(false);
    m_tooltipBg->setZValue(100);
    scene()->addItem(m_tooltipBg);
    
    // 创建提示文字
    m_tooltipText = new QGraphicsTextItem();
    m_tooltipText->setDefaultTextColor(Qt::black);
    QFont font;
    font.setPointSize(9);
    m_tooltipText->setFont(font);
    m_tooltipText->setVisible(false);
    m_tooltipText->setZValue(101);
    scene()->addItem(m_tooltipText);
}

InteractiveChartView::~InteractiveChartView()
{
    // QGraphicsScene 会自动删除 items
}

void InteractiveChartView::setAxes(QValueAxis *axisX, QValueAxis *axisY)
{
    m_axisX = axisX;
    m_axisY = axisY;
}

void InteractiveChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        m_lastMousePos = event->position().toPoint();
#else
        m_lastMousePos = event->pos();
#endif
        setCursor(Qt::ClosedHandCursor);
        hideCrosshair();  // 拖拽时隐藏十字线
    }
    QChartView::mousePressEvent(event);
}

void InteractiveChartView::mouseMoveEvent(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPoint currentPos = event->position().toPoint();
#else
    QPoint currentPos = event->pos();
#endif

    if (m_isDragging && m_axisX && m_axisY) {
        // 计算鼠标移动的像素差
        QPointF delta = currentPos - m_lastMousePos;
        m_lastMousePos = currentPos;
        
        // 获取当前轴范围
        double xMin = m_axisX->min();
        double xMax = m_axisX->max();
        double yMin = m_axisY->min();
        double yMax = m_axisY->max();
        
        // 获取图表绑定区域
        QRectF plotArea = chart()->plotArea();
        
        // 计算像素到数据的比例
        double xScale = (xMax - xMin) / plotArea.width();
        double yScale = (yMax - yMin) / plotArea.height();
        
        // 计算数据偏移量（向左拖动，数据向右移动，所以取负）
        double xOffset = -delta.x() * xScale;
        double yOffset = delta.y() * yScale;  // Y轴方向相反
        
        // 更新轴范围
        m_axisX->setRange(xMin + xOffset, xMax + xOffset);
        m_axisY->setRange(yMin + yOffset, yMax + yOffset);
    } else {
        // 非拖拽模式，更新十字线
        updateCrosshair(currentPos);
    }
    
    QChartView::mouseMoveEvent(event);
}

void InteractiveChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
    QChartView::mouseReleaseEvent(event);
}

void InteractiveChartView::leaveEvent(QEvent *event)
{
    QChartView::leaveEvent(event);
    hideCrosshair();
    if (m_isDragging) {
        m_isDragging = false;
        setCursor(Qt::ArrowCursor);
    }
}

void InteractiveChartView::wheelEvent(QWheelEvent *event)
{
    if (!m_axisX || !m_axisY) {
        QChartView::wheelEvent(event);
        return;
    }
    
    // 获取滚轮方向
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    int delta = event->angleDelta().y();
#else
    int delta = event->delta();
#endif
    
    // 缩放因子
    double factor = delta > 0 ? 0.8 : 1.25;
    
    // 获取鼠标位置对应的图表坐标
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QPointF mousePos = event->position();
    Qt::KeyboardModifiers modifiers = event->modifiers();
#elif QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QPointF mousePos = event->position();
    Qt::KeyboardModifiers modifiers = event->modifiers();
#else
    QPointF mousePos = event->posF();
    Qt::KeyboardModifiers modifiers = event->modifiers();
#endif
    QPointF chartPos = chart()->mapToValue(mousePos.toPoint());
    
    // 获取当前轴范围
    double xMin = m_axisX->min();
    double xMax = m_axisX->max();
    double yMin = m_axisY->min();
    double yMax = m_axisY->max();
    
    // 判断缩放模式：
    // Shift键：只缩放Y轴
    // Ctrl键：只缩放X轴
    // 无修饰键：同时缩放X和Y轴
    bool zoomX = !(modifiers & Qt::ShiftModifier);
    bool zoomY = !(modifiers & Qt::ControlModifier);
    
    // 以鼠标位置为中心进行缩放
    double xRange = (xMax - xMin) * factor;
    double yRange = (yMax - yMin) * factor;
    
    // 计算新的范围，保持鼠标位置不变
    double xRatio = (chartPos.x() - xMin) / (xMax - xMin);
    double yRatio = (chartPos.y() - yMin) / (yMax - yMin);
    
    double newXMin = chartPos.x() - xRange * xRatio;
    double newXMax = chartPos.x() + xRange * (1 - xRatio);
    double newYMin = chartPos.y() - yRange * yRatio;
    double newYMax = chartPos.y() + yRange * (1 - yRatio);
    
    if (zoomX) {
        m_axisX->setRange(newXMin, newXMax);
    }
    if (zoomY) {
        m_axisY->setRange(newYMin, newYMax);
    }
    
    event->accept();
}

void InteractiveChartView::updateCrosshair(const QPoint &pos)
{
    if (!m_axisX || !m_axisY || chart()->series().isEmpty()) {
        hideCrosshair();
        return;
    }
    
    // 获取图表绑定区域
    QRectF plotArea = chart()->plotArea();
    
    // 检查鼠标是否在绑定区域内
    if (!plotArea.contains(pos)) {
        hideCrosshair();
        return;
    }
    
    // 将像素坐标转换为图表坐标
    QPointF chartPos = chart()->mapToValue(pos);
    double xValue = chartPos.x();
    double yValue = chartPos.y();
    
    // 绘制垂直虚线
    m_verticalLine->setLine(pos.x(), plotArea.top(), pos.x(), plotArea.bottom());
    m_verticalLine->setVisible(true);
    
    // 绘制水平虚线
    m_horizontalLine->setLine(plotArea.left(), pos.y(), plotArea.right(), pos.y());
    m_horizontalLine->setVisible(true);
    
    // 构建提示文本
    QString tooltipStr = buildTooltipText(xValue, yValue);
    m_tooltipText->setHtml(tooltipStr);
    
    // 计算提示框位置和大小
    QRectF textRect = m_tooltipText->boundingRect();
    double tooltipX = pos.x() + 15;
    double tooltipY = pos.y() - textRect.height() / 2;
    
    // 确保提示框不超出视图边界
    if (tooltipX + textRect.width() + 10 > width()) {
        tooltipX = pos.x() - textRect.width() - 15;
    }
    if (tooltipY < 0) {
        tooltipY = 5;
    }
    if (tooltipY + textRect.height() > height()) {
        tooltipY = height() - textRect.height() - 5;
    }
    
    m_tooltipText->setPos(tooltipX + 5, tooltipY + 3);
    m_tooltipBg->setRect(tooltipX, tooltipY, textRect.width() + 10, textRect.height() + 6);
    
    m_tooltipText->setVisible(true);
    m_tooltipBg->setVisible(true);
}

void InteractiveChartView::hideCrosshair()
{
    m_verticalLine->setVisible(false);
    m_horizontalLine->setVisible(false);
    m_tooltipBg->setVisible(false);
    m_tooltipText->setVisible(false);
}

QString InteractiveChartView::buildTooltipText(double xValue, double yValue)
{
    QString html = QString("<b>X: %1 &nbsp; Y: %2</b><br>").arg(xValue, 0, 'f', 4).arg(yValue, 0, 'f', 4);
    html += "<table cellspacing='2'>";
    
    for (QAbstractSeries *abstractSeries : chart()->series()) {
        // 尝试作为 QLineSeries
        QLineSeries *lineSeries = qobject_cast<QLineSeries*>(abstractSeries);
        if (lineSeries) {
            double yValue = interpolateY(lineSeries, xValue);
            QColor color = lineSeries->color();
            
            html += QString("<tr>"
                            "<td><font color='%1'>●</font></td>"
                            "<td>%2:</td>"
                            "<td><b>%3</b></td>"
                            "</tr>")
                    .arg(color.name())
                    .arg(lineSeries->name())
                    .arg(yValue, 0, 'f', 4);
            continue;
        }
        
        // 尝试作为 QScatterSeries
        QScatterSeries *scatterSeries = qobject_cast<QScatterSeries*>(abstractSeries);
        if (scatterSeries) {
            // 对于散点图，找最近的点
            double yValue = interpolateYScatter(scatterSeries, xValue);
            QColor color = scatterSeries->color();
            
            html += QString("<tr>"
                            "<td><font color='%1'>●</font></td>"
                            "<td>%2:</td>"
                            "<td><b>%3</b></td>"
                            "</tr>")
                    .arg(color.name())
                    .arg(scatterSeries->name())
                    .arg(yValue, 0, 'f', 4);
        }
    }
    
    html += "</table>";
    return html;
}

double InteractiveChartView::interpolateY(QLineSeries *series, double xValue)
{
    QList<QPointF> points = series->points();
    if (points.isEmpty()) {
        return 0;
    }
    
    // 如果x值在范围外
    if (xValue <= points.first().x()) {
        return points.first().y();
    }
    if (xValue >= points.last().x()) {
        return points.last().y();
    }
    
    // 二分查找最近的点
    int left = 0;
    int right = points.size() - 1;
    
    while (right - left > 1) {
        int mid = (left + right) / 2;
        if (points[mid].x() <= xValue) {
            left = mid;
        } else {
            right = mid;
        }
    }
    
    // 线性插值
    double x0 = points[left].x();
    double x1 = points[right].x();
    double y0 = points[left].y();
    double y1 = points[right].y();
    
    if (x1 - x0 == 0) {
        return y0;
    }
    
    double t = (xValue - x0) / (x1 - x0);
    return y0 + t * (y1 - y0);
}

double InteractiveChartView::interpolateYScatter(QScatterSeries *series, double xValue)
{
    QList<QPointF> points = series->points();
    if (points.isEmpty()) {
        return 0;
    }
    
    // 对于散点图，找最接近xValue的点
    double minDist = std::numeric_limits<double>::max();
    double yValue = 0;
    
    for (const QPointF &point : points) {
        double dist = std::abs(point.x() - xValue);
        if (dist < minDist) {
            minDist = dist;
            yValue = point.y();
        }
    }
    
    return yValue;
}
