#include "chartwidget.h"
#include <QHBoxLayout>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QDialog>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QGridLayout>
#include <cmath>
#include <limits>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QT_USE_NAMESPACE
#else
QT_CHARTS_USE_NAMESPACE
#endif

// ==================== ChartWidget ====================

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_seriesCount(0)
    , m_originalXMin(0), m_originalXMax(1)
    , m_originalYMin(0), m_originalYMax(1)
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
    m_zoomInBtn->setText("🔍+");
    m_zoomInBtn->setToolTip("放大 (Ctrl+滚轮向上)");
    m_zoomInBtn->setFixedSize(32, 28);
    connect(m_zoomInBtn, &QToolButton::clicked, this, &ChartWidget::zoomIn);
    
    // 缩小按钮
    m_zoomOutBtn = new QToolButton();
    m_zoomOutBtn->setText("🔍-");
    m_zoomOutBtn->setToolTip("缩小 (Ctrl+滚轮向下)");
    m_zoomOutBtn->setFixedSize(32, 28);
    connect(m_zoomOutBtn, &QToolButton::clicked, this, &ChartWidget::zoomOut);
    
    // 重置按钮
    m_zoomResetBtn = new QToolButton();
    m_zoomResetBtn->setText("↺");
    m_zoomResetBtn->setToolTip("重置视图");
    m_zoomResetBtn->setFixedSize(32, 28);
    connect(m_zoomResetBtn, &QToolButton::clicked, this, &ChartWidget::zoomReset);
    
    toolLayout->addWidget(new QLabel("缩放:"));
    toolLayout->addWidget(m_zoomInBtn);
    toolLayout->addWidget(m_zoomOutBtn);
    toolLayout->addWidget(m_zoomResetBtn);
    
    // 分隔符
    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);
    toolLayout->addWidget(separator);
    
    // 标记按钮
    m_markerBtn = new QToolButton();
    m_markerBtn->setText("📏");
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
            series->attachAxis(m_axisY);
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
            series->attachAxis(m_axisY);
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
            lineSeries->attachAxis(m_axisY);
            
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
            scatterSeries->attachAxis(m_axisY);
            
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
    
    m_seriesCount++;
    
    // 更新坐标轴范围
    updateAxisRanges();
}

void ChartWidget::updateAxisRanges()
{
    if (m_chart->series().isEmpty()) {
        return;
    }
    
    double xMin = std::numeric_limits<double>::max();
    double xMax = std::numeric_limits<double>::lowest();
    double yMin = std::numeric_limits<double>::max();
    double yMax = std::numeric_limits<double>::lowest();
    
    for (QAbstractSeries *abstractSeries : m_chart->series()) {
        QLineSeries *series = qobject_cast<QLineSeries*>(abstractSeries);
        if (!series) continue;
        
        for (const QPointF &point : series->points()) {
            xMin = qMin(xMin, point.x());
            xMax = qMax(xMax, point.x());
            yMin = qMin(yMin, point.y());
            yMax = qMax(yMax, point.y());
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

void ChartWidget::clearChart()
{
    clearMarkerLines();
    m_markerInfos.clear();
    m_chart->removeAllSeries();
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

void ChartWidget::zoomIn()
{
    m_chart->zoomIn();
}

void ChartWidget::zoomOut()
{
    m_chart->zoomOut();
}

void ChartWidget::zoomReset()
{
    m_axisX->setRange(m_originalXMin, m_originalXMax);
    m_axisY->setRange(m_originalYMin, m_originalYMax);
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
    
    // 获取当前坐标轴范围用于绘制延伸线
    double axisXMin = m_axisX->min();
    double axisXMax = m_axisX->max();
    double axisYMin = m_axisY->min();
    double axisYMax = m_axisY->max();
    
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
#elif QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QPointF mousePos = event->position();
#else
    QPointF mousePos = event->posF();
#endif
    QPointF chartPos = chart()->mapToValue(mousePos.toPoint());
    
    // 获取当前轴范围
    double xMin = m_axisX->min();
    double xMax = m_axisX->max();
    double yMin = m_axisY->min();
    double yMax = m_axisY->max();
    
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
    
    m_axisX->setRange(newXMin, newXMax);
    m_axisY->setRange(newYMin, newYMax);
    
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
