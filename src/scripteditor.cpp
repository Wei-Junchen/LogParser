#include "scripteditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFont>
#include <QDateTime>

ScriptEditorDialog::ScriptEditorDialog(ScriptEngine *engine, CsvParser *parser, QWidget *parent)
    : QDialog(parent)
    , m_scriptEngine(engine)
    , m_csvParser(parser)
{
    setupUi();
    updateAvailableColumns();
    refreshDerivedColumnList();
    
    setWindowTitle("脚本编辑器 - 数据融合处理");
    resize(900, 700);
}

ScriptEditorDialog::~ScriptEditorDialog()
{
}

QList<DerivedColumn> ScriptEditorDialog::getDerivedColumns() const
{
    return m_scriptEngine->getDerivedColumns();
}

void ScriptEditorDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 主分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // ===== 左侧：脚本编辑区 =====
    QWidget *leftWidget = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // 工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    QLabel *columnLabel = new QLabel("插入列:");
    m_columnComboBox = new QComboBox();
    m_columnComboBox->setMinimumWidth(120);
    QPushButton *insertColumnBtn = new QPushButton("插入");
    connect(insertColumnBtn, &QPushButton::clicked, this, &ScriptEditorDialog::onInsertColumn);
    
    toolbarLayout->addWidget(columnLabel);
    toolbarLayout->addWidget(m_columnComboBox);
    toolbarLayout->addWidget(insertColumnBtn);
    
    toolbarLayout->addSpacing(20);
    
    QLabel *funcLabel = new QLabel("插入函数:");
    m_functionComboBox = new QComboBox();
    m_functionComboBox->addItem("math.add(a, b)", "math.add(data., data.)");
    m_functionComboBox->addItem("math.subtract(a, b)", "math.subtract(data., data.)");
    m_functionComboBox->addItem("math.multiply(a, b)", "math.multiply(data., data.)");
    m_functionComboBox->addItem("math.divide(a, b)", "math.divide(data., data.)");
    m_functionComboBox->addItem("math.scale(a, k)", "math.scale(data., 1.0)");
    m_functionComboBox->addItem("math.normalize(a)", "math.normalize(data.)");
    m_functionComboBox->addItem("math.standardize(a)", "math.standardize(data.)");
    m_functionComboBox->addItem("math.moving_average(a, n)", "math.moving_average(data., 5)");
    m_functionComboBox->addItem("math.lowpass_filter(a, c)", "math.lowpass_filter(data., 0.1)");
    m_functionComboBox->addItem("math.derivative(a, dt)", "math.derivative(data., 1.0)");
    m_functionComboBox->addItem("math.integral(a, dt)", "math.integral(data., 1.0)");
    m_functionComboBox->addItem("math.fft_magnitude(a)", "math.fft_magnitude(data.)");
    m_functionComboBox->addItem("math.power_spectrum(a)", "math.power_spectrum(data.)");
    m_functionComboBox->addItem("math.abs_array(a)", "math.abs_array(data.)");
    m_functionComboBox->addItem("math.sqrt_array(a)", "math.sqrt_array(data.)");
    m_functionComboBox->addItem("math.log_array(a)", "math.log_array(data.)");
    m_functionComboBox->addItem("math.correlation_coefficient(a, b)", "math.correlation_coefficient(data., data.)");
    
    QPushButton *insertFuncBtn = new QPushButton("插入");
    connect(insertFuncBtn, &QPushButton::clicked, this, &ScriptEditorDialog::onInsertFunction);
    
    toolbarLayout->addWidget(funcLabel);
    toolbarLayout->addWidget(m_functionComboBox);
    toolbarLayout->addWidget(insertFuncBtn);
    toolbarLayout->addStretch();
    
    leftLayout->addLayout(toolbarLayout);
    
    // 脚本编辑器
    QGroupBox *scriptGroup = new QGroupBox("脚本 (JavaScript)");
    QVBoxLayout *scriptLayout = new QVBoxLayout(scriptGroup);
    
    m_scriptEdit = new QPlainTextEdit();
    QFont monoFont("Monospace");
    monoFont.setStyleHint(QFont::Monospace);
    monoFont.setPointSize(10);
    m_scriptEdit->setFont(monoFont);
    m_scriptEdit->setPlaceholderText(
        "// 在此编写数据处理脚本\n"
        "// 使用 data.列名 访问CSV中的列\n"
        "// 使用 math.函数名() 调用内置函数\n"
        "// 脚本必须返回一个数组\n\n"
        "// 示例: 归一化温度数据\n"
        "return math.normalize(data.Temperature);"
    );
    m_scriptEdit->setMinimumHeight(200);
    scriptLayout->addWidget(m_scriptEdit);
    
    // 输出列名
    QHBoxLayout *outputNameLayout = new QHBoxLayout();
    outputNameLayout->addWidget(new QLabel("输出列名:"));
    m_outputNameEdit = new QLineEdit();
    m_outputNameEdit->setPlaceholderText("请输入新列的名称");
    outputNameLayout->addWidget(m_outputNameEdit);
    scriptLayout->addLayout(outputNameLayout);
    
    leftLayout->addWidget(scriptGroup);
    
    // 输出区
    QGroupBox *outputGroup = new QGroupBox("执行结果");
    QVBoxLayout *outputLayout = new QVBoxLayout(outputGroup);
    
    m_outputBrowser = new QTextBrowser();
    m_outputBrowser->setFont(monoFont);
    m_outputBrowser->setMaximumHeight(150);
    outputLayout->addWidget(m_outputBrowser);
    
    QHBoxLayout *outputBtnLayout = new QHBoxLayout();
    QPushButton *clearOutputBtn = new QPushButton("清除输出");
    connect(clearOutputBtn, &QPushButton::clicked, this, &ScriptEditorDialog::onClearOutput);
    outputBtnLayout->addWidget(clearOutputBtn);
    outputBtnLayout->addStretch();
    outputLayout->addLayout(outputBtnLayout);
    
    leftLayout->addWidget(outputGroup);
    
    // ===== 右侧：派生列列表 =====
    QWidget *rightWidget = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    
    QGroupBox *derivedGroup = new QGroupBox("已生成的派生列");
    QVBoxLayout *derivedLayout = new QVBoxLayout(derivedGroup);
    
    m_derivedListWidget = new QListWidget();
    connect(m_derivedListWidget, &QListWidget::itemClicked, 
            this, &ScriptEditorDialog::onDerivedColumnSelected);
    derivedLayout->addWidget(m_derivedListWidget);
    
    m_deleteButton = new QPushButton("删除选中列");
    m_deleteButton->setEnabled(false);
    connect(m_deleteButton, &QPushButton::clicked, this, &ScriptEditorDialog::onDeleteDerivedColumn);
    derivedLayout->addWidget(m_deleteButton);
    
    rightLayout->addWidget(derivedGroup);
    
    // 添加到分割器
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(rightWidget);
    mainSplitter->setStretchFactor(0, 2);
    mainSplitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(mainSplitter);
    
    // 底部按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_runButton = new QPushButton("运行脚本");
    m_runButton->setMinimumHeight(35);
    m_runButton->setStyleSheet("font-weight: bold; background-color: #4CAF50; color: white;");
    connect(m_runButton, &QPushButton::clicked, this, &ScriptEditorDialog::onRunScript);
    
    m_helpButton = new QPushButton("帮助文档");
    m_helpButton->setMinimumHeight(35);
    connect(m_helpButton, &QPushButton::clicked, this, &ScriptEditorDialog::onShowHelp);
    
    m_closeButton = new QPushButton("关闭");
    m_closeButton->setMinimumHeight(35);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    buttonLayout->addWidget(m_runButton);
    buttonLayout->addWidget(m_helpButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeButton);
    
    mainLayout->addLayout(buttonLayout);
}

void ScriptEditorDialog::updateAvailableColumns()
{
    m_columnComboBox->clear();
    
    if (m_csvParser) {
        QStringList columns = m_csvParser->getColumnNames();
        for (const QString &col : columns) {
            if (m_csvParser->isNumericColumn(columns.indexOf(col))) {
                m_columnComboBox->addItem(col, QString("data.%1").arg(col));
            }
        }
    }
    
    // 添加派生列
    for (const auto &derived : m_scriptEngine->getDerivedColumns()) {
        m_columnComboBox->addItem(QString("[派生] %1").arg(derived.name), 
                                  QString("data.%1").arg(derived.name));
    }
}

void ScriptEditorDialog::refreshDerivedColumnList()
{
    m_derivedListWidget->clear();
    
    for (const auto &derived : m_scriptEngine->getDerivedColumns()) {
        QListWidgetItem *item = new QListWidgetItem(derived.name);
        item->setToolTip(QString("脚本:\n%1").arg(derived.sourceScript));
        item->setData(Qt::UserRole, derived.name);
        m_derivedListWidget->addItem(item);
    }
    
    m_deleteButton->setEnabled(false);
}

void ScriptEditorDialog::appendOutput(const QString &text, bool isError)
{
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    QString color = isError ? "red" : "green";
    m_outputBrowser->append(QString("<span style='color:%1'>%2%3</span>")
        .arg(color).arg(timestamp).arg(text.toHtmlEscaped()));
}

void ScriptEditorDialog::onRunScript()
{
    QString script = m_scriptEdit->toPlainText().trimmed();
    QString outputName = m_outputNameEdit->text().trimmed();
    
    if (script.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入脚本内容");
        return;
    }
    
    if (outputName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入输出列名");
        return;
    }
    
    // 准备源数据
    if (m_csvParser) {
        QMap<QString, QVector<double>> sourceData;
        QStringList columns = m_csvParser->getColumnNames();
        for (int i = 0; i < columns.size(); ++i) {
            if (m_csvParser->isNumericColumn(i)) {
                sourceData[columns[i]] = m_csvParser->getColumnData(i);
            }
        }
        
        // 添加已有的派生列
        for (const auto &derived : m_scriptEngine->getDerivedColumns()) {
            sourceData[derived.name] = derived.data;
        }
        
        m_scriptEngine->setSourceData(sourceData);
    }
    
    // 执行脚本
    appendOutput(QString("正在执行脚本，输出列: %1").arg(outputName), false);
    
    if (m_scriptEngine->executeScript(script, outputName)) {
        QVector<double> result = m_scriptEngine->getDerivedColumnData(outputName);
        appendOutput(QString("成功！生成了 %1 个数据点").arg(result.size()), false);
        
        // 显示前几个值
        QString preview = "预览: [";
        int previewCount = qMin(5, result.size());
        for (int i = 0; i < previewCount; ++i) {
            preview += QString::number(result[i], 'g', 4);
            if (i < previewCount - 1) preview += ", ";
        }
        if (result.size() > 5) preview += ", ...";
        preview += "]";
        appendOutput(preview, false);
        
        refreshDerivedColumnList();
        updateAvailableColumns();
        
        emit derivedColumnsChanged();
    } else {
        appendOutput(QString("错误: %1").arg(m_scriptEngine->getLastError()), true);
    }
}

void ScriptEditorDialog::onClearOutput()
{
    m_outputBrowser->clear();
}

void ScriptEditorDialog::onShowHelp()
{
    QDialog helpDialog(this);
    helpDialog.setWindowTitle("脚本引擎帮助");
    helpDialog.resize(700, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(&helpDialog);
    
    QTextBrowser *browser = new QTextBrowser();
    QFont monoFont("Monospace");
    monoFont.setStyleHint(QFont::Monospace);
    browser->setFont(monoFont);
    browser->setPlainText(ScriptEngine::getHelpDocument());
    layout->addWidget(browser);
    
    QPushButton *closeBtn = new QPushButton("关闭");
    connect(closeBtn, &QPushButton::clicked, &helpDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    helpDialog.exec();
}

void ScriptEditorDialog::onInsertColumn()
{
    if (m_columnComboBox->currentIndex() >= 0) {
        QString code = m_columnComboBox->currentData().toString();
        m_scriptEdit->insertPlainText(code);
        m_scriptEdit->setFocus();
    }
}

void ScriptEditorDialog::onInsertFunction()
{
    if (m_functionComboBox->currentIndex() >= 0) {
        QString code = m_functionComboBox->currentData().toString();
        m_scriptEdit->insertPlainText(code);
        m_scriptEdit->setFocus();
    }
}

void ScriptEditorDialog::onDerivedColumnSelected(QListWidgetItem *item)
{
    m_deleteButton->setEnabled(item != nullptr);
    
    if (item) {
        QString name = item->data(Qt::UserRole).toString();
        for (const auto &derived : m_scriptEngine->getDerivedColumns()) {
            if (derived.name == name) {
                m_scriptEdit->setPlainText(derived.sourceScript);
                m_outputNameEdit->setText(derived.name);
                break;
            }
        }
    }
}

void ScriptEditorDialog::onDeleteDerivedColumn()
{
    QListWidgetItem *item = m_derivedListWidget->currentItem();
    if (!item) return;
    
    QString name = item->data(Qt::UserRole).toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除派生列 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_scriptEngine->removeDerivedColumn(name);
        refreshDerivedColumnList();
        updateAvailableColumns();
        emit derivedColumnsChanged();
        appendOutput(QString("已删除派生列: %1").arg(name), false);
    }
}
