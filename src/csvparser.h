#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>

/**
 * @brief CSV解析器类
 * 用于读取和解析CSV文件，存储列数据
 */
class CsvParser
{
public:
    CsvParser();
    
    /**
     * @brief 解析CSV文件
     * @param filePath 文件路径
     * @return 是否成功解析
     */
    bool parseFile(const QString &filePath);
    
    /**
     * @brief 获取所有列名
     * @return 列名列表
     */
    QStringList getColumnNames() const;
    
    /**
     * @brief 获取指定列的数据
     * @param columnName 列名
     * @return 该列的所有数据
     */
    QVector<double> getColumnData(const QString &columnName) const;
    
    /**
     * @brief 获取指定索引列的数据
     * @param columnIndex 列索引
     * @return 该列的所有数据
     */
    QVector<double> getColumnData(int columnIndex) const;
    
    /**
     * @brief 获取行数
     * @return 数据行数（不包含表头）
     */
    int getRowCount() const;
    
    /**
     * @brief 获取列数
     * @return 列数
     */
    int getColumnCount() const;
    
    /**
     * @brief 检查某列是否为数值列
     * @param columnIndex 列索引
     * @return 是否为数值列
     */
    bool isNumericColumn(int columnIndex) const;
    
    /**
     * @brief 获取错误信息
     * @return 最后一次错误的信息
     */
    QString getLastError() const;
    
    /**
     * @brief 清空数据
     */
    void clear();

private:
    QStringList m_columnNames;                      // 列名列表
    QVector<QVector<QString>> m_rawData;            // 原始字符串数据
    QMap<QString, QVector<double>> m_numericData;   // 数值数据缓存
    QString m_lastError;                            // 错误信息
    
    /**
     * @brief 解析单行CSV数据
     * @param line CSV行
     * @return 分割后的字段列表
     */
    QStringList parseLine(const QString &line);
    
    /**
     * @brief 尝试将字符串转换为数值
     * @param str 字符串
     * @param ok 转换是否成功
     * @return 转换后的数值
     */
    double toDouble(const QString &str, bool *ok = nullptr);
};

#endif // CSVPARSER_H
