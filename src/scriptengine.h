#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QVariant>
#include <QJSEngine>
#include <QJSValue>
#include <cmath>
#include <complex>
#include <algorithm>
#include <numeric>

/**
 * @brief 派生数据列结构
 */
struct DerivedColumn
{
    QString name;               // 列名
    QVector<double> data;       // 数据
    QString sourceScript;       // 生成该列的脚本
};

/**
 * @brief 脚本引擎类
 * 提供JavaScript脚本执行能力，支持数据处理和变换
 */
class ScriptEngine : public QObject
{
    Q_OBJECT
    
public:
    explicit ScriptEngine(QObject *parent = nullptr);
    ~ScriptEngine();
    
    /**
     * @brief 设置源数据列
     * @param columns 列名到数据的映射
     */
    void setSourceData(const QMap<QString, QVector<double>> &columns);
    
    /**
     * @brief 执行脚本生成新列
     * @param script 用户脚本
     * @param outputColumnName 输出列名
     * @return 是否成功
     */
    bool executeScript(const QString &script, const QString &outputColumnName);
    
    /**
     * @brief 获取生成的派生列
     */
    QList<DerivedColumn> getDerivedColumns() const { return m_derivedColumns; }
    
    /**
     * @brief 获取派生列数据
     */
    QVector<double> getDerivedColumnData(const QString &name) const;
    
    /**
     * @brief 清除所有派生列
     */
    void clearDerivedColumns();
    
    /**
     * @brief 删除指定派生列
     */
    bool removeDerivedColumn(const QString &name);
    
    /**
     * @brief 获取最后的错误信息
     */
    QString getLastError() const { return m_lastError; }
    
    /**
     * @brief 获取所有可用的列名（源列+派生列）
     */
    QStringList getAllColumnNames() const;
    
    /**
     * @brief 获取帮助文档
     */
    static QString getHelpDocument();

private:
    /**
     * @brief 注册内置函数到JS引擎
     */
    void registerBuiltinFunctions();
    
    /**
     * @brief 将QVector转换为JS数组
     */
    QJSValue vectorToJSArray(const QVector<double> &vec);
    
    /**
     * @brief 将JS数组转换为QVector
     */
    QVector<double> jsArrayToVector(const QJSValue &array);

private:
    QJSEngine *m_jsEngine;
    QMap<QString, QVector<double>> m_sourceData;
    QList<DerivedColumn> m_derivedColumns;
    QString m_lastError;
};

/**
 * @brief 数学工具类，提供给JS引擎使用
 */
class MathUtils : public QObject
{
    Q_OBJECT
    
public:
    explicit MathUtils(QObject *parent = nullptr) : QObject(parent) {}
    
    // ===== 基本数组操作 =====
    Q_INVOKABLE QVariantList add(const QVariantList &a, const QVariantList &b);
    Q_INVOKABLE QVariantList subtract(const QVariantList &a, const QVariantList &b);
    Q_INVOKABLE QVariantList multiply(const QVariantList &a, const QVariantList &b);
    Q_INVOKABLE QVariantList divide(const QVariantList &a, const QVariantList &b);
    Q_INVOKABLE QVariantList scale(const QVariantList &a, double factor);
    Q_INVOKABLE QVariantList offset(const QVariantList &a, double value);
    
    // ===== 统计函数 =====
    Q_INVOKABLE double sum(const QVariantList &a);
    Q_INVOKABLE double mean(const QVariantList &a);
    Q_INVOKABLE double std_dev(const QVariantList &a);
    Q_INVOKABLE double variance(const QVariantList &a);
    Q_INVOKABLE double min_val(const QVariantList &a);
    Q_INVOKABLE double max_val(const QVariantList &a);
    Q_INVOKABLE double rms(const QVariantList &a);
    
    // ===== 归一化 =====
    Q_INVOKABLE QVariantList normalize(const QVariantList &a);           // 0-1归一化
    Q_INVOKABLE QVariantList standardize(const QVariantList &a);         // Z-score标准化
    Q_INVOKABLE QVariantList normalize_range(const QVariantList &a, double newMin, double newMax);
    
    // ===== 滤波器 =====
    Q_INVOKABLE QVariantList moving_average(const QVariantList &a, int windowSize);
    Q_INVOKABLE QVariantList median_filter(const QVariantList &a, int windowSize);
    Q_INVOKABLE QVariantList lowpass_filter(const QVariantList &a, double cutoffRatio);
    Q_INVOKABLE QVariantList highpass_filter(const QVariantList &a, double cutoffRatio);
    
    // ===== 微积分 =====
    Q_INVOKABLE QVariantList derivative(const QVariantList &a, double dt = 1.0);
    Q_INVOKABLE QVariantList integral(const QVariantList &a, double dt = 1.0);
    Q_INVOKABLE QVariantList cumsum(const QVariantList &a);
    
    // ===== 傅里叶变换 =====
    Q_INVOKABLE QVariantList fft_magnitude(const QVariantList &a);       // FFT幅值
    Q_INVOKABLE QVariantList fft_phase(const QVariantList &a);           // FFT相位
    Q_INVOKABLE QVariantList fft_frequency(int n, double sampleRate);    // 频率轴
    Q_INVOKABLE QVariantList power_spectrum(const QVariantList &a);      // 功率谱
    
    // ===== 数学函数 =====
    Q_INVOKABLE QVariantList abs_array(const QVariantList &a);
    Q_INVOKABLE QVariantList sqrt_array(const QVariantList &a);
    Q_INVOKABLE QVariantList pow_array(const QVariantList &a, double exp);
    Q_INVOKABLE QVariantList log_array(const QVariantList &a);
    Q_INVOKABLE QVariantList log10_array(const QVariantList &a);
    Q_INVOKABLE QVariantList exp_array(const QVariantList &a);
    Q_INVOKABLE QVariantList sin_array(const QVariantList &a);
    Q_INVOKABLE QVariantList cos_array(const QVariantList &a);
    Q_INVOKABLE QVariantList tan_array(const QVariantList &a);
    
    // ===== 信号生成 =====
    Q_INVOKABLE QVariantList linspace(double start, double end, int n);
    Q_INVOKABLE QVariantList zeros(int n);
    Q_INVOKABLE QVariantList ones(int n);
    Q_INVOKABLE QVariantList sine_wave(int n, double frequency, double amplitude = 1.0, double phase = 0.0);
    
    // ===== 相关性分析 =====
    Q_INVOKABLE QVariantList cross_correlation(const QVariantList &a, const QVariantList &b);
    Q_INVOKABLE double correlation_coefficient(const QVariantList &a, const QVariantList &b);
    
    // ===== 插值与重采样 =====
    Q_INVOKABLE QVariantList resample(const QVariantList &a, int newSize);
    Q_INVOKABLE QVariantList interp_linear(const QVariantList &x, const QVariantList &y, const QVariantList &xNew);
    
    // ===== 时间转换 =====
    Q_INVOKABLE QVariantList ms_to_s_from_zero(const QVariantList &a);   // ms转s，从0开始
    Q_INVOKABLE QVariantList us_to_s_from_zero(const QVariantList &a);   // us转s，从0开始
    Q_INVOKABLE QVariantList ns_to_s_from_zero(const QVariantList &a);   // ns转s，从0开始
    Q_INVOKABLE QVariantList time_from_zero(const QVariantList &a, double scaleFactor = 1.0);  // 通用时间归零并缩放
    
private:
    QVector<double> toVector(const QVariantList &list);
    QVariantList toVariantList(const QVector<double> &vec);
    
    // FFT辅助函数
    void fft_impl(std::vector<std::complex<double>> &data, bool inverse = false);
};

#endif // SCRIPTENGINE_H
