#include "presetmanager.h"
#include <QDebug>
#include <QFileInfo>
#include <QRegularExpression>

PresetManager::PresetManager()
{
    // 确保预设目录存在
    QDir dir(getPresetsDirectory());
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    scanPresets();
}

QStringList PresetManager::getSchemeNames() const
{
    return m_schemeNames;
}

PresetScheme PresetManager::getScheme(const QString &name) const
{
    QString filePath = getSchemeFilePath(name);
    if (QFile::exists(filePath)) {
        return loadSchemeFromFile(filePath);
    }
    return PresetScheme();
}

bool PresetManager::saveScheme(const PresetScheme &scheme)
{
    if (scheme.name.isEmpty()) {
        return false;
    }
    
    QString filePath = getSchemeFilePath(scheme.name);
    bool success = saveSchemeToFile(scheme, filePath);
    
    if (success && !m_schemeNames.contains(scheme.name)) {
        m_schemeNames.append(scheme.name);
        m_schemeNames.sort();
    }
    
    return success;
}

bool PresetManager::deleteScheme(const QString &name)
{
    QString filePath = getSchemeFilePath(name);
    QFile file(filePath);
    
    if (file.exists() && file.remove()) {
        m_schemeNames.removeAll(name);
        return true;
    }
    
    return false;
}

bool PresetManager::renameScheme(const QString &oldName, const QString &newName)
{
    if (oldName == newName) {
        return true;
    }
    
    if (hasScheme(newName)) {
        return false;  // 新名称已存在
    }
    
    // 加载旧预设
    PresetScheme scheme = getScheme(oldName);
    if (scheme.name.isEmpty()) {
        return false;
    }
    
    // 更新名称并保存为新文件
    scheme.name = newName;
    if (!saveScheme(scheme)) {
        return false;
    }
    
    // 删除旧文件
    deleteScheme(oldName);
    
    return true;
}

bool PresetManager::hasScheme(const QString &name) const
{
    return m_schemeNames.contains(name);
}

void PresetManager::refresh()
{
    scanPresets();
}

bool PresetManager::exportScheme(const QString &name, const QString &filePath) const
{
    PresetScheme scheme = getScheme(name);
    if (scheme.name.isEmpty()) {
        return false;
    }
    
    return saveSchemeToFile(scheme, filePath);
}

QString PresetManager::importScheme(const QString &filePath, bool overwrite)
{
    PresetScheme scheme = loadSchemeFromFile(filePath);
    
    if (scheme.name.isEmpty()) {
        qWarning() << "导入预设失败: 文件格式无效或预设名称为空";
        return QString();
    }
    
    // 检查是否已存在同名预设
    if (hasScheme(scheme.name) && !overwrite) {
        // 自动生成新名称
        QString baseName = scheme.name;
        int counter = 1;
        while (hasScheme(scheme.name)) {
            scheme.name = QString("%1 (%2)").arg(baseName).arg(counter++);
        }
    }
    
    if (saveScheme(scheme)) {
        return scheme.name;
    }
    
    return QString();
}

QString PresetManager::getPresetsDirectory() const
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return configPath + "/presets";
}

QString PresetManager::getSchemeFilePath(const QString &name) const
{
    return getPresetsDirectory() + "/" + nameToFileName(name) + ".json";
}

void PresetManager::scanPresets()
{
    m_schemeNames.clear();
    
    QDir dir(getPresetsDirectory());
    if (!dir.exists()) {
        return;
    }
    
    QStringList filters;
    filters << "*.json";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files | QDir::Readable);
    
    for (const QFileInfo &fileInfo : files) {
        // 从文件内容中读取真实名称
        PresetScheme scheme = loadSchemeFromFile(fileInfo.absoluteFilePath());
        if (!scheme.name.isEmpty()) {
            m_schemeNames.append(scheme.name);
        }
    }
    
    m_schemeNames.sort();
}

PresetScheme PresetManager::loadSchemeFromFile(const QString &filePath) const
{
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开预设文件:" << filePath;
        return PresetScheme();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "解析预设文件失败:" << error.errorString() << "文件:" << filePath;
        return PresetScheme();
    }
    
    return PresetScheme::fromJson(doc.object());
}

bool PresetManager::saveSchemeToFile(const PresetScheme &scheme, const QString &filePath) const
{
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
    
    QJsonDocument doc(scheme.toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}

QString PresetManager::nameToFileName(const QString &name) const
{
    // 将预设名称转换为安全的文件名
    // 替换不安全的字符
    QString safeName = name;
    static QRegularExpression unsafeChars(R"([<>:"/\\|?*])");
    safeName.replace(unsafeChars, "_");
    
    // 限制长度
    if (safeName.length() > 100) {
        safeName = safeName.left(100);
    }
    
    return safeName;
}

QString PresetManager::fileNameToName(const QString &fileName) const
{
    // 移除 .json 后缀
    QString name = fileName;
    if (name.endsWith(".json", Qt::CaseInsensitive)) {
        name = name.left(name.length() - 5);
    }
    return name;
}
