#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>
#include <QSize>
#include <QPoint>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>

/**
 * @brief 应用程序惰性配置类
 * 保存用户的使用习惯和上次使用的配置
 * 与预设方案配置分离，存储在独立的配置文件中
 */
class AppSettings
{
public:
    static AppSettings& instance();
    
    // 禁止拷贝
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;
    
    // ==================== 文件路径相关 ====================
    
    /**
     * @brief 获取上次打开文件的目录
     */
    QString lastOpenDirectory() const;
    
    /**
     * @brief 设置上次打开文件的目录
     */
    void setLastOpenDirectory(const QString &path);
    
    /**
     * @brief 获取上次保存图表的目录
     */
    QString lastSaveDirectory() const;
    
    /**
     * @brief 设置上次保存图表的目录
     */
    void setLastSaveDirectory(const QString &path);
    
    /**
     * @brief 获取上次导入预设的目录
     */
    QString lastPresetImportDirectory() const;
    
    /**
     * @brief 设置上次导入预设的目录
     */
    void setLastPresetImportDirectory(const QString &path);
    
    /**
     * @brief 获取上次导出预设的目录
     */
    QString lastPresetExportDirectory() const;
    
    /**
     * @brief 设置上次导出预设的目录
     */
    void setLastPresetExportDirectory(const QString &path);
    
    // ==================== 预设相关 ====================
    
    /**
     * @brief 获取上次使用的预设名称
     */
    QString lastUsedPreset() const;
    
    /**
     * @brief 设置上次使用的预设名称
     */
    void setLastUsedPreset(const QString &presetName);
    
    // ==================== 窗口状态相关 ====================
    
    /**
     * @brief 获取窗口大小
     */
    QSize windowSize() const;
    
    /**
     * @brief 设置窗口大小
     */
    void setWindowSize(const QSize &size);
    
    /**
     * @brief 获取窗口位置
     */
    QPoint windowPosition() const;
    
    /**
     * @brief 设置窗口位置
     */
    void setWindowPosition(const QPoint &pos);
    
    /**
     * @brief 获取窗口是否最大化
     */
    bool isWindowMaximized() const;
    
    /**
     * @brief 设置窗口是否最大化
     */
    void setWindowMaximized(bool maximized);
    
    // ==================== 其他设置 ====================
    
    /**
     * @brief 获取最近打开的文件列表
     */
    QStringList recentFiles() const;
    
    /**
     * @brief 添加最近打开的文件
     */
    void addRecentFile(const QString &filePath);
    
    /**
     * @brief 清除最近打开的文件列表
     */
    void clearRecentFiles();
    
    /**
     * @brief 获取最大最近文件数量
     */
    int maxRecentFiles() const;
    
    /**
     * @brief 同步设置到磁盘
     */
    void sync();

private:
    AppSettings();
    ~AppSettings();
    
    QString getSettingsFilePath() const;
    
    QSettings *m_settings;
    
    // 默认值
    static const int DEFAULT_MAX_RECENT_FILES = 10;
};

#endif // APPSETTINGS_H
