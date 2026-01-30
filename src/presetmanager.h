#ifndef PRESETMANAGER_H
#define PRESETMANAGER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include "seriesstyledialog.h"

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
    QMap<QString, SeriesStyle> seriesStyles;  // 曲线样式设置
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["xAxisColumn"] = xAxisColumn;
        obj["yAxisColumns"] = QJsonArray::fromStringList(yAxisColumns);
        obj["computedColumns"] = QJsonArray::fromStringList(computedColumns);
        
        // 保存曲线样式
        QJsonObject stylesObj;
        for (auto it = seriesStyles.constBegin(); it != seriesStyles.constEnd(); ++it) {
            stylesObj[it.key()] = it.value().toJson();
        }
        obj["seriesStyles"] = stylesObj;
        
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
        
        // 加载曲线样式
        QJsonObject stylesObj = obj["seriesStyles"].toObject();
        for (auto it = stylesObj.constBegin(); it != stylesObj.constEnd(); ++it) {
            preset.seriesStyles[it.key()] = SeriesStyle::fromJson(it.value().toObject());
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
    QString description;                // 方案描述（可选）
    QString version;                    // 版本号
    QList<ScriptPreset> scripts;        // 脚本列表（先执行）
    QList<PlotPreset> canvasPresets;    // 各Canvas的预设
    
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["description"] = description;
        obj["version"] = version.isEmpty() ? "1.0" : version;
        
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
        scheme.description = obj["description"].toString();
        scheme.version = obj["version"].toString("1.0");
        
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
 * 每个预设方案保存为独立的JSON文件，便于跨机导入导出
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
     * @brief 重新扫描预设目录，刷新预设列表
     */
    void refresh();
    
    /**
     * @brief 导出预设到指定文件
     * @param name 预设名称
     * @param filePath 导出文件路径
     * @return 成功返回true
     */
    bool exportScheme(const QString &name, const QString &filePath) const;
    
    /**
     * @brief 从文件导入预设
     * @param filePath 导入文件路径
     * @param overwrite 如果同名预设已存在，是否覆盖
     * @return 成功返回导入的预设名称，失败返回空字符串
     */
    QString importScheme(const QString &filePath, bool overwrite = false);
    
    /**
     * @brief 获取预设目录路径
     */
    QString getPresetsDirectory() const;
    
    /**
     * @brief 获取指定预设的文件路径
     */
    QString getSchemeFilePath(const QString &name) const;

private:
    /**
     * @brief 扫描预设目录，加载所有预设名称
     */
    void scanPresets();
    
    /**
     * @brief 从文件加载预设
     */
    PresetScheme loadSchemeFromFile(const QString &filePath) const;
    
    /**
     * @brief 保存预设到文件
     */
    bool saveSchemeToFile(const PresetScheme &scheme, const QString &filePath) const;
    
    /**
     * @brief 将名称转换为安全的文件名
     */
    QString nameToFileName(const QString &name) const;
    
    /**
     * @brief 从文件名提取预设名称
     */
    QString fileNameToName(const QString &fileName) const;
    
    // 预设名称列表（不加载完整内容，按需加载）
    QStringList m_schemeNames;
};

#endif // PRESETMANAGER_H
