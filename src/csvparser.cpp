#include "csvparser.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>

CsvParser::CsvParser()
{
}

bool CsvParser::parseFile(const QString &filePath)
{
    clear();
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("无法打开文件: %1").arg(file.errorString());
        return false;
    }
    
    QTextStream in(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    in.setCodec("UTF-8");
#else
    in.setEncoding(QStringConverter::Utf8);
#endif
    
    // 读取表头
    if (in.atEnd()) {
        m_lastError = "文件为空";
        file.close();
        return false;
    }
    
    QString headerLine = in.readLine();
    m_columnNames = parseLine(headerLine);
    
    if (m_columnNames.isEmpty()) {
        m_lastError = "无法解析表头";
        file.close();
        return false;
    }
    
    // 读取数据行
    int lineNumber = 1;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;
        
        if (line.trimmed().isEmpty()) {
            continue;  // 跳过空行
        }
        
        QStringList fields = parseLine(line);
        
        // 如果字段数少于列数，补充空字符串
        while (fields.size() < m_columnNames.size()) {
            fields.append("");
        }
        
        // 如果字段数多于列数，截断
        if (fields.size() > m_columnNames.size()) {
            fields = fields.mid(0, m_columnNames.size());
        }
        
        m_rawData.append(fields.toVector());
    }
    
    file.close();
    
    // 预处理数值列
    for (int col = 0; col < m_columnNames.size(); ++col) {
        if (isNumericColumn(col)) {
            QVector<double> numericCol;
            for (int row = 0; row < m_rawData.size(); ++row) {
                bool ok;
                double value = toDouble(m_rawData[row][col], &ok);
                numericCol.append(ok ? value : 0.0);
            }
            m_numericData[m_columnNames[col]] = numericCol;
        }
    }
    
    return true;
}

QStringList CsvParser::getColumnNames() const
{
    return m_columnNames;
}

QVector<double> CsvParser::getColumnData(const QString &columnName) const
{
    if (m_numericData.contains(columnName)) {
        return m_numericData[columnName];
    }
    return QVector<double>();
}

QVector<double> CsvParser::getColumnData(int columnIndex) const
{
    if (columnIndex >= 0 && columnIndex < m_columnNames.size()) {
        return getColumnData(m_columnNames[columnIndex]);
    }
    return QVector<double>();
}

int CsvParser::getRowCount() const
{
    return m_rawData.size();
}

int CsvParser::getColumnCount() const
{
    return m_columnNames.size();
}

bool CsvParser::isNumericColumn(int columnIndex) const
{
    if (columnIndex < 0 || columnIndex >= m_columnNames.size()) {
        return false;
    }
    
    // 检查该列中至少有一半的值可以转换为数值
    int numericCount = 0;
    int totalCount = 0;
    
    for (int row = 0; row < m_rawData.size() && row < 100; ++row) {  // 只检查前100行
        QString value = m_rawData[row][columnIndex].trimmed();
        if (value.isEmpty()) {
            continue;
        }
        totalCount++;
        
        // 直接尝试转换，不调用成员函数
        QString cleaned = value;
        cleaned.remove(',');
        cleaned.remove(' ');
        bool ok;
        cleaned.toDouble(&ok);
        if (ok) {
            numericCount++;
        }
    }
    
    // 如果超过70%的非空值是数值，认为是数值列
    return totalCount > 0 && (double)numericCount / totalCount >= 0.7;
}

QString CsvParser::getLastError() const
{
    return m_lastError;
}

void CsvParser::clear()
{
    m_columnNames.clear();
    m_rawData.clear();
    m_numericData.clear();
    m_lastError.clear();
}

QStringList CsvParser::parseLine(const QString &line)
{
    QStringList fields;
    QString field;
    bool inQuotes = false;
    
    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        
        if (c == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                // 转义的引号
                field += '"';
                i++;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (c == ',' && !inQuotes) {
            fields.append(field.trimmed());
            field.clear();
        } else {
            field += c;
        }
    }
    
    // 添加最后一个字段
    fields.append(field.trimmed());
    
    return fields;
}

double CsvParser::toDouble(const QString &str, bool *ok)
{
    QString cleaned = str.trimmed();
    
    // 移除千位分隔符
    cleaned.remove(',');
    cleaned.remove(' ');
    
    // 尝试直接转换
    bool convertOk;
    double value = cleaned.toDouble(&convertOk);
    
    if (ok) {
        *ok = convertOk;
    }
    
    return value;
}
