#include "seriesstyledialog.h"
#include <QFormLayout>
#include <QFrame>

SeriesStyleDialog::SeriesStyleDialog(const QString &seriesName,
                                     const SeriesStyle &currentStyle,
                                     QWidget *parent)
    : QDialog(parent)
    , m_seriesName(seriesName)
{
    setWindowTitle(QString("曲线样式设置 - %1").arg(seriesName));
    setMinimumWidth(350);
    
    setupUi();
    applyStyle(currentStyle);
}

void SeriesStyleDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // ========== 显示模式 ==========
    QGroupBox *modeGroup = new QGroupBox("显示模式");
    QFormLayout *modeLayout = new QFormLayout(modeGroup);
    
    m_displayModeCombo = new QComboBox();
    m_displayModeCombo->addItem("连线模式", static_cast<int>(SeriesDisplayMode::Line));
    m_displayModeCombo->addItem("散点模式", static_cast<int>(SeriesDisplayMode::Scatter));
    m_displayModeCombo->addItem("连线 + 散点", static_cast<int>(SeriesDisplayMode::LineAndScatter));
    connect(m_displayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SeriesStyleDialog::onDisplayModeChanged);
    modeLayout->addRow("模式:", m_displayModeCombo);
    
    m_lineWidthSpin = new QSpinBox();
    m_lineWidthSpin->setRange(1, 10);
    m_lineWidthSpin->setValue(2);
    m_lineWidthSpin->setSuffix(" px");
    modeLayout->addRow("线宽:", m_lineWidthSpin);
    
    mainLayout->addWidget(modeGroup);
    
    // ========== 散点设置 ==========
    m_scatterGroup = new QGroupBox("散点设置");
    QFormLayout *scatterLayout = new QFormLayout(m_scatterGroup);
    
    m_scatterSizeSpin = new QSpinBox();
    m_scatterSizeSpin->setRange(2, 20);
    m_scatterSizeSpin->setValue(6);
    m_scatterSizeSpin->setSuffix(" px");
    scatterLayout->addRow("散点大小:", m_scatterSizeSpin);
    
    mainLayout->addWidget(m_scatterGroup);
    
    // ========== Y值区间过滤 ==========
    m_rangeGroup = new QGroupBox("Y值区间过滤");
    m_rangeGroup->setCheckable(true);
    m_rangeGroup->setChecked(false);
    QFormLayout *rangeLayout = new QFormLayout(m_rangeGroup);
    
    m_minValueSpin = new QDoubleSpinBox();
    m_minValueSpin->setRange(-1e12, 1e12);
    m_minValueSpin->setDecimals(6);
    m_minValueSpin->setValue(0.0);
    rangeLayout->addRow("最小值:", m_minValueSpin);
    
    m_maxValueSpin = new QDoubleSpinBox();
    m_maxValueSpin->setRange(-1e12, 1e12);
    m_maxValueSpin->setDecimals(6);
    m_maxValueSpin->setValue(100.0);
    rangeLayout->addRow("最大值:", m_maxValueSpin);
    
    QLabel *rangeHint = new QLabel("仅绘制Y值在此区间内的数据点");
    rangeHint->setStyleSheet("color: gray; font-size: 10px;");
    rangeLayout->addRow("", rangeHint);
    
    mainLayout->addWidget(m_rangeGroup);
    
    // ========== Y轴分组 ==========
    QGroupBox *axisGroup = new QGroupBox("Y轴分组（多Y轴模式）");
    QFormLayout *axisLayout = new QFormLayout(axisGroup);
    
    m_yAxisGroupSpin = new QSpinBox();
    m_yAxisGroupSpin->setRange(0, 9);
    m_yAxisGroupSpin->setValue(0);
    m_yAxisGroupSpin->setToolTip("相同组号的系列将共享同一个Y轴\n组号0-9，默认每个系列独立Y轴（自动分配）");
    axisLayout->addRow("Y轴组号:", m_yAxisGroupSpin);
    
    QLabel *axisHint = new QLabel("相同组号的曲线共享Y轴，0表示独立");
    axisHint->setStyleSheet("color: gray; font-size: 10px;");
    axisLayout->addRow("", axisHint);
    
    mainLayout->addWidget(axisGroup);
    
    // ========== 按钮 ==========
    mainLayout->addSpacing(10);
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    QPushButton *resetBtn = new QPushButton("恢复默认");
    connect(resetBtn, &QPushButton::clicked, this, &SeriesStyleDialog::onResetToDefault);
    buttonLayout->addWidget(resetBtn);
    
    buttonLayout->addStretch();
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonLayout->addWidget(buttonBox);
    
    mainLayout->addLayout(buttonLayout);
    
    // 初始化散点组的可见性
    onDisplayModeChanged(0);
}

void SeriesStyleDialog::applyStyle(const SeriesStyle &style)
{
    // 设置显示模式
    int modeIndex = m_displayModeCombo->findData(static_cast<int>(style.displayMode));
    if (modeIndex >= 0) {
        m_displayModeCombo->setCurrentIndex(modeIndex);
    }
    
    // 设置线宽
    m_lineWidthSpin->setValue(style.lineWidth);
    
    // 设置散点大小
    m_scatterSizeSpin->setValue(style.scatterSize);
    
    // 设置区间过滤
    m_rangeGroup->setChecked(style.filterByRange);
    m_minValueSpin->setValue(style.minValue);
    m_maxValueSpin->setValue(style.maxValue);
    
    // 设置Y轴分组
    m_yAxisGroupSpin->setValue(style.yAxisGroup);
}

SeriesStyle SeriesStyleDialog::getStyle() const
{
    SeriesStyle style;
    
    style.displayMode = static_cast<SeriesDisplayMode>(
        m_displayModeCombo->currentData().toInt());
    style.lineWidth = m_lineWidthSpin->value();
    style.scatterSize = m_scatterSizeSpin->value();
    style.filterByRange = m_rangeGroup->isChecked();
    style.minValue = m_minValueSpin->value();
    style.maxValue = m_maxValueSpin->value();
    style.yAxisGroup = m_yAxisGroupSpin->value();
    
    return style;
}

void SeriesStyleDialog::onFilterByRangeChanged(bool checked)
{
    m_minValueSpin->setEnabled(checked);
    m_maxValueSpin->setEnabled(checked);
}

void SeriesStyleDialog::onDisplayModeChanged(int index)
{
    SeriesDisplayMode mode = static_cast<SeriesDisplayMode>(
        m_displayModeCombo->itemData(index).toInt());
    
    // 散点设置仅在散点模式或连线+散点模式下可用
    bool showScatter = (mode == SeriesDisplayMode::Scatter || 
                        mode == SeriesDisplayMode::LineAndScatter);
    m_scatterGroup->setEnabled(showScatter);
    
    // 线宽仅在连线模式或连线+散点模式下可用
    bool showLine = (mode == SeriesDisplayMode::Line || 
                     mode == SeriesDisplayMode::LineAndScatter);
    m_lineWidthSpin->setEnabled(showLine);
}

void SeriesStyleDialog::onResetToDefault()
{
    applyStyle(SeriesStyle());  // 使用默认构造的样式
}
