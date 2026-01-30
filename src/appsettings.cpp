#include "appsettings.h"
#include <QFileInfo>

AppSettings& AppSettings::instance()
{
    static AppSettings instance;
    return instance;
}

AppSettings::AppSettings()
{
    QString settingsPath = getSettingsFilePath();
    
    // 确保目录存在
    QFileInfo fileInfo(settingsPath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    m_settings = new QSettings(settingsPath, QSettings::IniFormat);
}

AppSettings::~AppSettings()
{
    if (m_settings) {
        m_settings->sync();
        delete m_settings;
    }
}

QString AppSettings::getSettingsFilePath() const
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return configPath + "/logparser_settings.ini";
}

// ==================== 文件路径相关 ====================

QString AppSettings::lastOpenDirectory() const
{
    return m_settings->value("Paths/LastOpenDirectory", 
                             QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
}

void AppSettings::setLastOpenDirectory(const QString &path)
{
    QFileInfo info(path);
    QString dir = info.isDir() ? path : info.absolutePath();
    m_settings->setValue("Paths/LastOpenDirectory", dir);
    m_settings->sync();
}

QString AppSettings::lastSaveDirectory() const
{
    return m_settings->value("Paths/LastSaveDirectory", 
                             QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
}

void AppSettings::setLastSaveDirectory(const QString &path)
{
    QFileInfo info(path);
    QString dir = info.isDir() ? path : info.absolutePath();
    m_settings->setValue("Paths/LastSaveDirectory", dir);
    m_settings->sync();
}

QString AppSettings::lastPresetImportDirectory() const
{
    return m_settings->value("Paths/LastPresetImportDirectory", 
                             QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
}

void AppSettings::setLastPresetImportDirectory(const QString &path)
{
    QFileInfo info(path);
    QString dir = info.isDir() ? path : info.absolutePath();
    m_settings->setValue("Paths/LastPresetImportDirectory", dir);
    m_settings->sync();
}

QString AppSettings::lastPresetExportDirectory() const
{
    return m_settings->value("Paths/LastPresetExportDirectory", 
                             QStandardPaths::writableLocation(QStandardPaths::HomeLocation)).toString();
}

void AppSettings::setLastPresetExportDirectory(const QString &path)
{
    QFileInfo info(path);
    QString dir = info.isDir() ? path : info.absolutePath();
    m_settings->setValue("Paths/LastPresetExportDirectory", dir);
    m_settings->sync();
}

// ==================== 预设相关 ====================

QString AppSettings::lastUsedPreset() const
{
    return m_settings->value("Presets/LastUsedPreset", "").toString();
}

void AppSettings::setLastUsedPreset(const QString &presetName)
{
    m_settings->setValue("Presets/LastUsedPreset", presetName);
    m_settings->sync();
}

// ==================== 窗口状态相关 ====================

QSize AppSettings::windowSize() const
{
    return m_settings->value("Window/Size", QSize(1200, 800)).toSize();
}

void AppSettings::setWindowSize(const QSize &size)
{
    m_settings->setValue("Window/Size", size);
    m_settings->sync();
}

QPoint AppSettings::windowPosition() const
{
    return m_settings->value("Window/Position", QPoint(100, 100)).toPoint();
}

void AppSettings::setWindowPosition(const QPoint &pos)
{
    m_settings->setValue("Window/Position", pos);
    m_settings->sync();
}

bool AppSettings::isWindowMaximized() const
{
    return m_settings->value("Window/Maximized", false).toBool();
}

void AppSettings::setWindowMaximized(bool maximized)
{
    m_settings->setValue("Window/Maximized", maximized);
    m_settings->sync();
}

// ==================== 其他设置 ====================

QStringList AppSettings::recentFiles() const
{
    return m_settings->value("RecentFiles/List", QStringList()).toStringList();
}

void AppSettings::addRecentFile(const QString &filePath)
{
    QStringList files = recentFiles();
    
    // 移除已存在的相同路径
    files.removeAll(filePath);
    
    // 添加到开头
    files.prepend(filePath);
    
    // 限制数量
    while (files.size() > maxRecentFiles()) {
        files.removeLast();
    }
    
    m_settings->setValue("RecentFiles/List", files);
    m_settings->sync();
}

void AppSettings::clearRecentFiles()
{
    m_settings->setValue("RecentFiles/List", QStringList());
    m_settings->sync();
}

int AppSettings::maxRecentFiles() const
{
    return m_settings->value("RecentFiles/MaxCount", DEFAULT_MAX_RECENT_FILES).toInt();
}

void AppSettings::sync()
{
    m_settings->sync();
}
