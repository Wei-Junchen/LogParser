#ifndef PRESETMANAGER_H
#define PRESETMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

/**
 * @brief 脚本预设结构
 * 保存一个派生列的脚本配置
 */
struct ScriptPreset
{
    QString outputName;             // 输出列名
    QString script;                 // 脚本内容
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["outputName"] = outputName;
        obj["script"] = script;
        return obj;
    }
    
    static ScriptPreset fromJson(const QJsonObject &obj) {
        ScriptPreset preset;
        preset.outputName = obj["outputName"].toString();
        preset.script = obj["script"].toString();
        return preset;
    }
};

/**
 * @brief 绘图预设结构
 * 保存一个Canvas的绘图配置
 */
struct PlotPreset
{
    QString name;                   // 预设名称
    QString xAxisColumn;            // X轴列名（空表示使用行索引）
    QStringList yAxisColumns;       // Y轴列名列表
    QStringList computedColumns;    // 选中的计算列列表
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["xAxisColumn"] = xAxisColumn;
        obj["yAxisColumns"] = QJsonArray::fromStringList(yAxisColumns);
        obj["computedColumns"] = QJsonArray::fromStringList(computedColumns);
        return obj;
    }
    
    static PlotPreset fromJson(const QJsonObject &obj) {
        PlotPreset preset;
        preset.name = obj["name"].toString();
        preset.xAxisColumn = obj["xAxisColumn"].toString();
        QJsonArray arr = obj["yAxisColumns"].toArray();
        for (const auto &val : arr) {
            preset.yAxisColumns.append(val.toString());
        }
        QJsonArray compArr = obj["computedColumns"].toArray();
        for (const auto &val : compArr) {
            preset.computedColumns.append(val.toString());
        }
        return preset;
    }
};

/**
 * @brief 预设方案结构
 * 包含多个Canvas预设的完整方案
 */
struct PresetScheme
{
    QString name;                       // 方案名称
    QList<ScriptPreset> scripts;        // 脚本列表（先执行）
    QList<PlotPreset> canvasPresets;    // 各Canvas的预设
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        
        // 保存脚本
        QJsonArray scriptArray;
        for (const auto &script : scripts) {
            scriptArray.append(script.toJson());
        }
        obj["scripts"] = scriptArray;
        
        // 保存Canvas预设
        QJsonArray canvasArray;
        for (const auto &preset : canvasPresets) {
            canvasArray.append(preset.toJson());
        }
        obj["canvases"] = canvasArray;
        return obj;
    }
    
    static PresetScheme fromJson(const QJsonObject &obj) {
        PresetScheme scheme;
        scheme.name = obj["name"].toString();
        
        // 加载脚本
        QJsonArray scriptArr = obj["scripts"].toArray();
        for (const auto &val : scriptArr) {
            scheme.scripts.append(ScriptPreset::fromJson(val.toObject()));
        }
        
        // 加载Canvas预设
        QJsonArray arr = obj["canvases"].toArray();
        for (const auto &val : arr) {
            scheme.canvasPresets.append(PlotPreset::fromJson(val.toObject()));
        }
        return scheme;
    }
};

/**
 * @brief 预设管理器类
 * 负责保存、加载和管理绘图预设
 */
class PresetManager
{
public:
    PresetManager();
    
    /**
     * @brief 获取所有预设方案名称
     */
    QStringList getSchemeNames() const;
    
    /**
     * @brief 获取指定预设方案
     */
    PresetScheme getScheme(const QString &name) const;
    
    /**
     * @brief 保存预设方案
     */
    bool saveScheme(const PresetScheme &scheme);
    
    /**
     * @brief 删除预设方案
     */
    bool deleteScheme(const QString &name);
    
    /**
     * @brief 重命名预设方案
     */
    bool renameScheme(const QString &oldName, const QString &newName);
    
    /**
     * @brief 检查预设方案是否存在
     */
    bool hasScheme(const QString &name) const;
    
    /**
     * @brief 加载所有预设
     */
    void loadPresets();
    
    /**
     * @brief 保存所有预设到文件
     */
    bool saveToFile();

private:
    /**
     * @brief 获取预设文件路径
     */
    QString getPresetsFilePath() const;
    
    QList<PresetScheme> m_schemes;
};

#endif // PRESETMANAGER_H
