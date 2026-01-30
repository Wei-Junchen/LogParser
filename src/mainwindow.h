#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTabBar>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QComboBox>
#include <QMenu>
#include "csvparser.h"
#include "canvaspanel.h"
#include "presetmanager.h"
#include "scriptengine.h"
#include "scripteditor.h"

/**
 * @brief 主窗口类
 * 管理CSV文件加载和多个Canvas
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    /**
     * @brief 打开CSV文件
     */
    void onOpenFile();
    
    /**
     * @brief 添加新Canvas
     */
    void onAddCanvas();
    
    /**
     * @brief 删除当前Canvas
     */
    void onRemoveCanvas();
    
    /**
     * @brief 保存当前图表
     */
    void onSaveChart();
    
    /**
     * @brief 打开脚本编辑器
     */
    void onOpenScriptEditor();
    
    /**
     * @brief 保存当前配置为预设
     */
    void onSavePreset();
    
    /**
     * @brief 加载预设
     */
    void onLoadPreset();
    
    /**
     * @brief 删除预设
     */
    void onDeletePreset();

private:
    /**
     * @brief 初始化UI
     */
    void setupUi();
    
    /**
     * @brief 创建菜单栏
     */
    void createMenuBar();
    
    /**
     * @brief 创建工具栏
     */
    void createToolBar();
    
    /**
     * @brief 刷新所有Canvas的列列表
     */
    void refreshAllCanvases();
    
    /**
     * @brief 获取当前Canvas
     */
    CanvasPanel* getCurrentCanvas();
    
    /**
     * @brief 刷新预设下拉框
     */
    void refreshPresetComboBox();
    
    /**
     * @brief 重命名Canvas
     */
    void renameCanvas(int index);
    
    /**
     * @brief 显示Canvas右键菜单
     */
    void showCanvasContextMenu(int index, const QPoint &globalPos);

private:
    // CSV解析器
    CsvParser m_csvParser;
    QString m_currentFilePath;
    
    // 预设管理器
    PresetManager m_presetManager;
    
    // 脚本引擎
    ScriptEngine *m_scriptEngine;
    
    // UI组件
    QTabWidget *m_canvasTabWidget;
    QComboBox *m_presetComboBox;
    
    // 状态栏标签
    QLabel *m_statusLabel;
    
    // Canvas计数器
    int m_canvasCounter;
};

#endif // MAINWINDOW_H
