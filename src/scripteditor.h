#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include <QDialog>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QTextBrowser>
#include <QComboBox>
#include "scriptengine.h"
#include "csvparser.h"

/**
 * @brief 脚本编辑器对话框
 * 允许用户编写和测试数据处理脚本
 */
class ScriptEditorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ScriptEditorDialog(ScriptEngine *engine, CsvParser *parser, QWidget *parent = nullptr);
    ~ScriptEditorDialog();
    
    /**
     * @brief 获取派生列列表
     */
    QList<DerivedColumn> getDerivedColumns() const;

signals:
    /**
     * @brief 派生列更新信号
     */
    void derivedColumnsChanged();

private slots:
    void onRunScript();
    void onClearOutput();
    void onShowHelp();
    void onInsertColumn();
    void onInsertFunction();
    void onDerivedColumnSelected(QListWidgetItem *item);
    void onDeleteDerivedColumn();

private:
    void setupUi();
    void refreshDerivedColumnList();
    void updateAvailableColumns();
    void appendOutput(const QString &text, bool isError = false);

private:
    ScriptEngine *m_scriptEngine;
    CsvParser *m_csvParser;
    
    // 脚本编辑区
    QPlainTextEdit *m_scriptEdit;
    QLineEdit *m_outputNameEdit;
    
    // 列选择
    QComboBox *m_columnComboBox;
    QComboBox *m_functionComboBox;
    
    // 输出区
    QTextBrowser *m_outputBrowser;
    
    // 派生列列表
    QListWidget *m_derivedListWidget;
    
    // 按钮
    QPushButton *m_runButton;
    QPushButton *m_helpButton;
    QPushButton *m_deleteButton;
    QPushButton *m_closeButton;
};

#endif // SCRIPTEDITOR_H
