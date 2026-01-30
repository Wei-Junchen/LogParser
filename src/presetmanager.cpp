#include "presetmanager.h"
#include <QDebug>

PresetManager::PresetManager()
{
    loadPresets();
}

QStringList PresetManager::getSchemeNames() const
{
    QStringList names;
    for (const auto &scheme : m_schemes) {
        names.append(scheme.name);
    }
    return names;
}

PresetScheme PresetManager::getScheme(const QString &name) const
{
    for (const auto &scheme : m_schemes) {
        if (scheme.name == name) {
            return scheme;
        }
    }
    return PresetScheme();
}

bool PresetManager::saveScheme(const PresetScheme &scheme)
{
    // 如果已存在同名方案，则替换
    for (int i = 0; i < m_schemes.size(); ++i) {
        if (m_schemes[i].name == scheme.name) {
            m_schemes[i] = scheme;
            return saveToFile();
        }
    }
    
    // 添加新方案
    m_schemes.append(scheme);
    return saveToFile();
}

bool PresetManager::deleteScheme(const QString &name)
{
    for (int i = 0; i < m_schemes.size(); ++i) {
        if (m_schemes[i].name == name) {
            m_schemes.removeAt(i);
            return saveToFile();
        }
    }
    return false;
}

bool PresetManager::renameScheme(const QString &oldName, const QString &newName)
{
    for (int i = 0; i < m_schemes.size(); ++i) {
        if (m_schemes[i].name == oldName) {
            m_schemes[i].name = newName;
            return saveToFile();
        }
    }
    return false;
}

bool PresetManager::hasScheme(const QString &name) const
{
    for (const auto &scheme : m_schemes) {
        if (scheme.name == name) {
            return true;
        }
    }
    return false;
}

void PresetManager::loadPresets()
{
    m_schemes.clear();
    
    QString filePath = getPresetsFilePath();
    QFile file(filePath);
    
    if (!file.exists()) {
        return;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开预设文件:" << filePath;
        return;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "解析预设文件失败:" << error.errorString();
        return;
    }
    
    QJsonArray arr = doc.array();
    for (const auto &val : arr) {
        m_schemes.append(PresetScheme::fromJson(val.toObject()));
    }
}

bool PresetManager::saveToFile()
{
    QString filePath = getPresetsFilePath();
    
    // 确保目录存在
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法写入预设文件:" << filePath;
        return false;
    }
    
    QJsonArray arr;
    for (const auto &scheme : m_schemes) {
        arr.append(scheme.toJson());
    }
    
    QJsonDocument doc(arr);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

QString PresetManager::getPresetsFilePath() const
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return configPath + "/logparser_presets.json";
}
