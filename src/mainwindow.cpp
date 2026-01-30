#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QApplication>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_canvasCounter(0)
    , m_statusLabel(nullptr)
    , m_scriptEngine(nullptr)
    , m_recentFilesMenu(nullptr)
{
    // 创建脚本引擎
    m_scriptEngine = new ScriptEngine(this);
    
    // 先创建状态栏标签
    m_statusLabel = new QLabel("请先打开CSV文件");
    statusBar()->addWidget(m_statusLabel);
    
    setupUi();
    createMenuBar();
    createToolBar();
    
    setWindowTitle("LogParser - CSV数据可视化工具");
    
    // 恢复窗口状态
    restoreWindowState();
    
    // 自动选择上次使用的预设
    QString lastPreset = AppSettings::instance().lastUsedPreset();
    if (!lastPreset.isEmpty() && m_presetManager.hasScheme(lastPreset)) {
        m_presetComboBox->setCurrentText(lastPreset);
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // 创建Canvas标签页容器
    m_canvasTabWidget = new QTabWidget(this);
    m_canvasTabWidget->setTabsClosable(true);
    m_canvasTabWidget->setMovable(true);
    
    // 关闭标签页
    connect(m_canvasTabWidget, &QTabWidget::tabCloseRequested, [this](int index) {
        if (m_canvasTabWidget->count() <= 1) {
            QMessageBox::warning(this, "警告", "至少需要保留一个Canvas");
            return;
        }
        QWidget *widget = m_canvasTabWidget->widget(index);
        m_canvasTabWidget->removeTab(index);
        delete widget;
    });
    
    // 双击标签页改名
    connect(m_canvasTabWidget, &QTabWidget::tabBarDoubleClicked, [this](int index) {
        if (index >= 0) {
            renameCanvas(index);
        }
    });
    
    // 右键菜单
    m_canvasTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_canvasTabWidget->tabBar(), &QWidget::customContextMenuRequested, 
            [this](const QPoint &pos) {
        int index = m_canvasTabWidget->tabBar()->tabAt(pos);
        if (index >= 0) {
            showCanvasContextMenu(index, m_canvasTabWidget->tabBar()->mapToGlobal(pos));
        }
    });
    
    setCentralWidget(m_canvasTabWidget);
    
    // 添加一个默认Canvas
    onAddCanvas();
}

void MainWindow::createMenuBar()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
    
    QAction *openAction = new QAction("打开CSV(&O)", this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    fileMenu->addAction(openAction);
    
    // 最近打开的文件子菜单
    m_recentFilesMenu = fileMenu->addMenu("最近打开的文件");
    updateRecentFilesMenu();
    
    fileMenu->addSeparator();
    
    QAction *saveAction = new QAction("保存图表(&S)", this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveChart);
    fileMenu->addAction(saveAction);
    
    fileMenu->addSeparator();
    
    // 预设导入导出
    QAction *importPresetAction = new QAction("导入预设(&I)...", this);
    connect(importPresetAction, &QAction::triggered, this, &MainWindow::onImportPreset);
    fileMenu->addAction(importPresetAction);
    
    QAction *exportPresetAction = new QAction("导出预设(&E)...", this);
    connect(exportPresetAction, &QAction::triggered, this, &MainWindow::onExportPreset);
    fileMenu->addAction(exportPresetAction);
    
    QAction *resetAllAction = new QAction("重置所有设置(&R)", this);
    connect(resetAllAction, &QAction::triggered, this, &MainWindow::onResetAllSettings);
    fileMenu->addAction(resetAllAction);

    fileMenu->addSeparator();
    
    QAction *exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);
    fileMenu->addAction(exitAction);
    
    // Canvas菜单
    QMenu *canvasMenu = menuBar()->addMenu("Canvas(&C)");
    
    QAction *addCanvasAction = new QAction("添加Canvas(&A)", this);
    addCanvasAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    connect(addCanvasAction, &QAction::triggered, this, &MainWindow::onAddCanvas);
    canvasMenu->addAction(addCanvasAction);
    
    QAction *removeCanvasAction = new QAction("删除当前Canvas(&D)", this);
    removeCanvasAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
    connect(removeCanvasAction, &QAction::triggered, this, &MainWindow::onRemoveCanvas);
    canvasMenu->addAction(removeCanvasAction);
    
    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    
    QAction *aboutAction = new QAction("关于(&A)", this);
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于 LogParser",
            "LogParser - CSV数据可视化工具\n\n"
            "版本：2.0\n\n"
            "使用方法：\n"
            "1. 打开CSV文件\n"
            "2. 添加/删除Canvas标签页\n"
            "3. 在每个Canvas中点击列名即可添加/移除曲线\n"
            "4. 可选择不同的X轴数据源");
    });
    helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBar()
{
    QToolBar *toolBar = addToolBar("主工具栏");
    toolBar->setMovable(false);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // 打开文件
    QAction *openAction = toolBar->addAction("打开CSV");
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    
    toolBar->addSeparator();
    
    // 添加Canvas
    QAction *addCanvasAction = toolBar->addAction("添加Canvas");
    connect(addCanvasAction, &QAction::triggered, this, &MainWindow::onAddCanvas);
    
    // 删除Canvas
    QAction *removeCanvasAction = toolBar->addAction("删除Canvas");
    connect(removeCanvasAction, &QAction::triggered, this, &MainWindow::onRemoveCanvas);
    
    toolBar->addSeparator();
    
    // 脚本编辑器
    QAction *scriptAction = toolBar->addAction("脚本编辑器");
    scriptAction->setToolTip("打开脚本编辑器，进行数据融合处理");
    connect(scriptAction, &QAction::triggered, this, &MainWindow::onOpenScriptEditor);
    
    toolBar->addSeparator();
    
    // 保存图表
    QAction *saveAction = toolBar->addAction("保存图表");
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveChart);
    
    toolBar->addSeparator();
    
    // 预设管理
    QLabel *presetLabel = new QLabel("预设方案: ");
    toolBar->addWidget(presetLabel);
    
    m_presetComboBox = new QComboBox();
    m_presetComboBox->setMinimumWidth(150);
    m_presetComboBox->setPlaceholderText("选择预设...");
    toolBar->addWidget(m_presetComboBox);
    refreshPresetComboBox();
    
    QAction *loadPresetAction = toolBar->addAction("加载预设");
    connect(loadPresetAction, &QAction::triggered, this, &MainWindow::onLoadPreset);
    
    QAction *savePresetAction = toolBar->addAction("保存预设");
    connect(savePresetAction, &QAction::triggered, this, &MainWindow::onSavePreset);
    
    QAction *deletePresetAction = toolBar->addAction("删除预设");
    connect(deletePresetAction, &QAction::triggered, this, &MainWindow::onDeletePreset);
}

void MainWindow::onOpenFile()
{
    // 使用上次打开的目录
    QString lastDir = AppSettings::instance().lastOpenDirectory();
    
    QString filePath = QFileDialog::getOpenFileName(this, 
        "打开CSV文件", 
        lastDir,
        "所有文件 (*)");
        // "CSV文件 (*.csv);;所有文件 (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 保存目录到惰性配置
    AppSettings::instance().setLastOpenDirectory(filePath);
    
    // 添加到最近文件
    AppSettings::instance().addRecentFile(filePath);
    updateRecentFilesMenu();
    
    m_statusLabel->setText("正在加载文件...");
    QApplication::processEvents();
    
    if (m_csvParser.parseFile(filePath)) {
        m_currentFilePath = filePath;
        
        // 刷新所有Canvas
        refreshAllCanvases();
        
        QFileInfo fileInfo(filePath);
        m_statusLabel->setText(QString("已加载: %1 (%2行 x %3列)")
            .arg(fileInfo.fileName())
            .arg(m_csvParser.getRowCount())
            .arg(m_csvParser.getColumnCount()));
    } else {
        QMessageBox::critical(this, "错误", 
            QString("无法解析文件：\n%1").arg(m_csvParser.getLastError()));
        m_statusLabel->setText("加载失败");
    }
}

void MainWindow::onAddCanvas()
{
    m_canvasCounter++;
    QString title = QString("Canvas %1").arg(m_canvasCounter);
    
    CanvasPanel *canvas = new CanvasPanel(&m_csvParser, m_scriptEngine, this);
    canvas->setTitle(title);
    
    m_canvasTabWidget->addTab(canvas, title);
    m_canvasTabWidget->setCurrentWidget(canvas);
    
    m_statusLabel->setText(QString("已添加 %1").arg(title));
    refreshAllCanvases();
}

void MainWindow::onRemoveCanvas()
{
    int currentIndex = m_canvasTabWidget->currentIndex();
    
    if (m_canvasTabWidget->count() <= 1) {
        QMessageBox::warning(this, "警告", "至少需要保留一个Canvas");
        return;
    }
    
    QWidget *widget = m_canvasTabWidget->widget(currentIndex);
    m_canvasTabWidget->removeTab(currentIndex);
    delete widget;
    
    m_statusLabel->setText("已删除Canvas");
}

void MainWindow::onOpenScriptEditor()
{
    if (m_csvParser.getColumnCount() == 0) {
        QMessageBox::warning(this, "警告", "请先打开CSV文件");
        return;
    }
    
    ScriptEditorDialog dialog(m_scriptEngine, &m_csvParser, this);
    
    connect(&dialog, &ScriptEditorDialog::derivedColumnsChanged, [this]() {
        // 刷新所有Canvas以显示新的派生列
        refreshAllCanvases();
    });
    
    dialog.exec();
    
    // 对话框关闭后刷新Canvas
    refreshAllCanvases();
    m_statusLabel->setText(QString("派生列数量: %1").arg(m_scriptEngine->getDerivedColumns().size()));
}

void MainWindow::onSaveChart()
{
    CanvasPanel *canvas = getCurrentCanvas();
    if (!canvas) {
        QMessageBox::warning(this, "警告", "没有可保存的Canvas");
        return;
    }
    
    // 使用上次保存的目录
    QString lastDir = AppSettings::instance().lastSaveDirectory();
    
    QString filePath = QFileDialog::getSaveFileName(this,
        "保存图表",
        lastDir + "/chart.png",
        "PNG图片 (*.png);;JPEG图片 (*.jpg);;所有文件 (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 保存目录到惰性配置
    AppSettings::instance().setLastSaveDirectory(filePath);
    
    if (canvas->getChart()->saveAsImage(filePath)) {
        m_statusLabel->setText(QString("图表已保存至：%1").arg(filePath));
    } else {
        QMessageBox::critical(this, "错误", "保存图表失败");
    }
}

void MainWindow::refreshAllCanvases()
{
    // 获取所有派生列
    QList<DerivedColumn> derivedColumns = m_scriptEngine->getDerivedColumns();
    
    for (int i = 0; i < m_canvasTabWidget->count(); ++i) {
        CanvasPanel *canvas = qobject_cast<CanvasPanel*>(m_canvasTabWidget->widget(i));
        if (canvas) {
            // 先清除旧的计算列
            canvas->clearComputedColumns();
            
            // 添加新的计算列到内部存储
            for (const DerivedColumn &col : derivedColumns) {
                canvas->addComputedColumn(col.name, col.data);
            }
            
            // 刷新列列表（会从 m_computedColumns 读取并显示）
            canvas->refreshColumnList();
        }
    }
}

CanvasPanel* MainWindow::getCurrentCanvas()
{
    return qobject_cast<CanvasPanel*>(m_canvasTabWidget->currentWidget());
}

void MainWindow::refreshPresetComboBox()
{
    m_presetComboBox->clear();
    QStringList names = m_presetManager.getSchemeNames();
    m_presetComboBox->addItems(names);
    
    if (names.isEmpty()) {
        m_presetComboBox->setPlaceholderText("无预设");
    } else {
        m_presetComboBox->setPlaceholderText("选择预设...");
    }
}

void MainWindow::onSavePreset()
{
    // 检查是否有Canvas
    if (m_canvasTabWidget->count() == 0) {
        QMessageBox::warning(this, "警告", "没有可保存的Canvas配置");
        return;
    }
    
    // 获取预设名称
    bool ok;
    QString name = QInputDialog::getText(this, "保存预设",
        "请输入预设方案名称:", QLineEdit::Normal, "", &ok);
    
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }
    
    name = name.trimmed();
    
    // 检查是否已存在
    if (m_presetManager.hasScheme(name)) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "确认",
            QString("预设方案 \"%1\" 已存在，是否覆盖？").arg(name),
            QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    
    // 收集所有Canvas的预设
    PresetScheme scheme;
    scheme.name = name;
    
    // 保存脚本（派生列）
    QList<DerivedColumn> derivedColumns = m_scriptEngine->getDerivedColumns();
    for (const DerivedColumn &col : derivedColumns) {
        ScriptPreset scriptPreset;
        scriptPreset.outputName = col.name;
        scriptPreset.script = col.sourceScript;
        scheme.scripts.append(scriptPreset);
    }
    
    // 保存Canvas预设
    for (int i = 0; i < m_canvasTabWidget->count(); ++i) {
        CanvasPanel *canvas = qobject_cast<CanvasPanel*>(m_canvasTabWidget->widget(i));
        if (canvas) {
            PlotPreset preset = canvas->getPreset();
            preset.name = m_canvasTabWidget->tabText(i);
            scheme.canvasPresets.append(preset);
        }
    }
    
    // 保存
    if (m_presetManager.saveScheme(scheme)) {
        refreshPresetComboBox();
        m_presetComboBox->setCurrentText(name);
        
        // 记录上次使用的预设
        AppSettings::instance().setLastUsedPreset(name);
        
        m_statusLabel->setText(QString("预设方案 \"%1\" 已保存 (含 %2 个脚本)")
            .arg(name).arg(scheme.scripts.size()));
    } else {
        QMessageBox::critical(this, "错误", "保存预设方案失败");
    }
}

void MainWindow::onLoadPreset()
{
    QString name = m_presetComboBox->currentText();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个预设方案");
        return;
    }
    
    if (!m_presetManager.hasScheme(name)) {
        QMessageBox::warning(this, "警告", QString("预设方案 \"%1\" 不存在").arg(name));
        return;
    }
    
    if (m_csvParser.getColumnCount() == 0) {
        QMessageBox::warning(this, "警告", "请先打开CSV文件");
        return;
    }
    
    PresetScheme scheme = m_presetManager.getScheme(name);
    
    // 1. 先执行脚本生成派生列
    // 清除现有派生列
    QList<DerivedColumn> existingCols = m_scriptEngine->getDerivedColumns();
    for (const DerivedColumn &col : existingCols) {
        m_scriptEngine->removeDerivedColumn(col.name);
    }
    
    // 设置源数据
    QMap<QString, QVector<double>> sourceData;
    QStringList columnNames = m_csvParser.getColumnNames();
    for (int i = 0; i < columnNames.size(); ++i) {
        if (m_csvParser.isNumericColumn(i)) {
            sourceData[columnNames[i]] = m_csvParser.getColumnData(i);
        }
    }
    m_scriptEngine->setSourceData(sourceData);
    
    // 执行预设中的脚本
    int scriptSuccess = 0;
    for (const ScriptPreset &scriptPreset : scheme.scripts) {
        if (m_scriptEngine->executeScript(scriptPreset.script, scriptPreset.outputName)) {
            scriptSuccess++;
        } else {
            qWarning() << "脚本执行失败:" << scriptPreset.outputName 
                       << m_scriptEngine->getLastError();
        }
    }
    
    // 2. 刷新所有Canvas的派生列
    refreshAllCanvases();
    
    // 3. 清除现有Canvas并创建新的
    while (m_canvasTabWidget->count() > 0) {
        QWidget *widget = m_canvasTabWidget->widget(0);
        m_canvasTabWidget->removeTab(0);
        delete widget;
    }
    
    m_canvasCounter = 0;
    
    // 4. 根据预设创建Canvas并应用配置
    for (const PlotPreset &preset : scheme.canvasPresets) {
        m_canvasCounter++;
        QString title = preset.name.isEmpty() ? 
            QString("Canvas %1").arg(m_canvasCounter) : preset.name;
        
        CanvasPanel *canvas = new CanvasPanel(&m_csvParser, m_scriptEngine, this);
        m_canvasTabWidget->addTab(canvas, title);
        canvas->setTitle(title);
        
        // 先添加派生列到Canvas内部数据
        QList<DerivedColumn> derivedColumns = m_scriptEngine->getDerivedColumns();
        for (const DerivedColumn &col : derivedColumns) {
            canvas->addComputedColumn(col.name, col.data);
        }
        
        // 刷新列列表（显示CSV列和计算列）
        canvas->refreshColumnList();
        
        // 再应用预设
        canvas->applyPreset(preset);
    }
    
    // 如果没有预设的Canvas，至少创建一个
    if (m_canvasTabWidget->count() == 0) {
        onAddCanvas();
    }
    
    // 记录上次使用的预设
    AppSettings::instance().setLastUsedPreset(name);
    
    m_statusLabel->setText(QString("已加载预设方案 \"%1\" (执行了 %2/%3 个脚本)")
        .arg(name).arg(scriptSuccess).arg(scheme.scripts.size()));
}

void MainWindow::onDeletePreset()
{
    QString name = m_presetComboBox->currentText();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个预设方案");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除预设方案 \"%1\" 吗？").arg(name),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    if (m_presetManager.deleteScheme(name)) {
        refreshPresetComboBox();
        m_statusLabel->setText(QString("预设方案 \"%1\" 已删除").arg(name));
    } else {
        QMessageBox::critical(this, "错误", "删除预设方案失败");
    }
}

void MainWindow::renameCanvas(int index)
{
    if (index < 0 || index >= m_canvasTabWidget->count()) {
        return;
    }
    
    QString currentName = m_canvasTabWidget->tabText(index);
    
    bool ok;
    QString newName = QInputDialog::getText(this, "重命名Canvas",
        "请输入新名称:", QLineEdit::Normal, currentName, &ok);
    
    if (ok && !newName.trimmed().isEmpty()) {
        newName = newName.trimmed();
        m_canvasTabWidget->setTabText(index, newName);
        
        // 同时更新Canvas的图表标题
        CanvasPanel *canvas = qobject_cast<CanvasPanel*>(m_canvasTabWidget->widget(index));
        if (canvas) {
            canvas->setTitle(newName);
        }
        
        m_statusLabel->setText(QString("Canvas已重命名为 \"%1\"").arg(newName));
    }
}

void MainWindow::showCanvasContextMenu(int index, const QPoint &globalPos)
{
    QMenu menu(this);
    
    QAction *renameAction = menu.addAction("重命名");
    connect(renameAction, &QAction::triggered, [this, index]() {
        renameCanvas(index);
    });
    
    QAction *clearAction = menu.addAction("清除曲线");
    connect(clearAction, &QAction::triggered, [this, index]() {
        CanvasPanel *canvas = qobject_cast<CanvasPanel*>(m_canvasTabWidget->widget(index));
        if (canvas) {
            canvas->clearChart();
            m_statusLabel->setText("已清除Canvas曲线");
        }
    });
    
    menu.addSeparator();
    
    QAction *deleteAction = menu.addAction("删除Canvas");
    deleteAction->setEnabled(m_canvasTabWidget->count() > 1);
    connect(deleteAction, &QAction::triggered, [this, index]() {
        if (m_canvasTabWidget->count() <= 1) {
            QMessageBox::warning(this, "警告", "至少需要保留一个Canvas");
            return;
        }
        QWidget *widget = m_canvasTabWidget->widget(index);
        m_canvasTabWidget->removeTab(index);
        delete widget;
        m_statusLabel->setText("已删除Canvas");
    });
    
    menu.exec(globalPos);
}

void MainWindow::onImportPreset()
{
    // 使用上次导入的目录
    QString lastDir = AppSettings::instance().lastPresetImportDirectory();
    
    QString filePath = QFileDialog::getOpenFileName(this,
        "导入预设",
        lastDir,
        "JSON预设文件 (*.json);;所有文件 (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 保存目录到惰性配置
    AppSettings::instance().setLastPresetImportDirectory(filePath);
    
    QString importedName = m_presetManager.importScheme(filePath, false);
    
    if (!importedName.isEmpty()) {
        refreshPresetComboBox();
        m_presetComboBox->setCurrentText(importedName);
        m_statusLabel->setText(QString("已导入预设方案 \"%1\"").arg(importedName));
        
        QMessageBox::information(this, "导入成功",
            QString("预设方案 \"%1\" 已成功导入").arg(importedName));
    } else {
        QMessageBox::critical(this, "导入失败",
            "无法导入预设文件，请检查文件格式是否正确");
    }
}

void MainWindow::onExportPreset()
{
    QString name = m_presetComboBox->currentText();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个预设方案");
        return;
    }
    
    // 使用上次导出的目录
    QString lastDir = AppSettings::instance().lastPresetExportDirectory();
    
    // 使用预设名称作为默认文件名
    QString defaultFileName = lastDir + "/" + name + ".json";
    
    QString filePath = QFileDialog::getSaveFileName(this,
        "导出预设",
        defaultFileName,
        "JSON预设文件 (*.json);;所有文件 (*)");
    
    if (filePath.isEmpty()) {
        return;
    }
    
    // 保存目录到惰性配置
    AppSettings::instance().setLastPresetExportDirectory(filePath);
    
    if (m_presetManager.exportScheme(name, filePath)) {
        m_statusLabel->setText(QString("预设方案 \"%1\" 已导出至 %2").arg(name, filePath));
        QMessageBox::information(this, "导出成功",
            QString("预设方案 \"%1\" 已成功导出到：\n%2").arg(name, filePath));
    } else {
        QMessageBox::critical(this, "导出失败", "无法导出预设文件");
    }
}

void MainWindow::saveWindowState()
{
    AppSettings &settings = AppSettings::instance();
    
    if (isMaximized()) {
        settings.setWindowMaximized(true);
    } else {
        settings.setWindowMaximized(false);
        settings.setWindowSize(size());
        settings.setWindowPosition(pos());
    }
    
    settings.sync();
}

void MainWindow::restoreWindowState()
{
    AppSettings &settings = AppSettings::instance();
    
    if (settings.isWindowMaximized()) {
        showMaximized();
    } else {
        resize(settings.windowSize());
        move(settings.windowPosition());
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveWindowState();
    event->accept();
}

void MainWindow::updateRecentFilesMenu()
{
    m_recentFilesMenu->clear();
    
    QStringList recentFiles = AppSettings::instance().recentFiles();
    
    if (recentFiles.isEmpty()) {
        QAction *emptyAction = m_recentFilesMenu->addAction("(无)");
        emptyAction->setEnabled(false);
        return;
    }
    
    for (const QString &filePath : recentFiles) {
        QFileInfo info(filePath);
        QString displayName = info.fileName();
        
        QAction *action = m_recentFilesMenu->addAction(displayName);
        action->setData(filePath);
        action->setToolTip(filePath);
        
        connect(action, &QAction::triggered, [this, filePath]() {
            openRecentFile(filePath);
        });
    }
    
    m_recentFilesMenu->addSeparator();
    
    QAction *clearAction = m_recentFilesMenu->addAction("清除最近打开记录");
    connect(clearAction, &QAction::triggered, [this]() {
        AppSettings::instance().clearRecentFiles();
        updateRecentFilesMenu();
        m_statusLabel->setText("已清除最近打开的文件记录");
    });
}

void MainWindow::openRecentFile(const QString &filePath)
{
    QFileInfo info(filePath);
    if (!info.exists()) {
        QMessageBox::warning(this, "文件不存在",
            QString("文件不存在或已被移动：\n%1").arg(filePath));
        return;
    }
    
    m_statusLabel->setText("正在加载文件...");
    QApplication::processEvents();
    
    if (m_csvParser.parseFile(filePath)) {
        m_currentFilePath = filePath;
        
        // 保存目录到惰性配置
        AppSettings::instance().setLastOpenDirectory(filePath);
        
        // 添加到最近文件（移到最前）
        AppSettings::instance().addRecentFile(filePath);
        updateRecentFilesMenu();
        
        // 刷新所有Canvas
        refreshAllCanvases();
        
        QFileInfo fileInfo(filePath);
        m_statusLabel->setText(QString("已加载: %1 (%2行 x %3列)")
            .arg(fileInfo.fileName())
            .arg(m_csvParser.getRowCount())
            .arg(m_csvParser.getColumnCount()));
    } else {
        QMessageBox::critical(this, "错误", 
            QString("无法解析文件：\n%1").arg(m_csvParser.getLastError()));
        m_statusLabel->setText("加载失败");
    }
}

void MainWindow::onResetAllSettings()
{
    QMessageBox::StandardButton reply = QMessageBox::warning(this, "确认重置",
        "此操作将删除所有用户设置和预设配置，包括：\n\n"
        "• 惰性配置（窗口位置、最近打开的文件等）\n"
        "• 所有保存的预设方案\n\n"
        "重置后程序将关闭，下次启动时将使用默认设置。\n\n"
        "确定要继续吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    // 获取配置目录路径
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir configDir(configPath);
    
    if (configDir.exists()) {
        // 删除整个配置目录
        if (configDir.removeRecursively()) {
            QMessageBox::information(this, "重置完成",
                QString("已成功删除配置目录：\n%1\n\n程序将关闭。").arg(configPath));
            
            // 关闭程序（不保存当前状态）
            QApplication::quit();
        } else {
            QMessageBox::critical(this, "重置失败",
                QString("无法删除配置目录：\n%1\n\n请检查文件权限或手动删除。").arg(configPath));
        }
    } else {
        QMessageBox::information(this, "无需重置",
            "配置目录不存在，无需重置。");
    }
}
