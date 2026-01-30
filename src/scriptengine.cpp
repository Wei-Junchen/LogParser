#include "scriptengine.h"
#include <QDebug>
#include <QtMath>

// ==================== ScriptEngine ====================

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent)
{
    m_jsEngine = new QJSEngine(this);
    registerBuiltinFunctions();
}

ScriptEngine::~ScriptEngine()
{
}

void ScriptEngine::setSourceData(const QMap<QString, QVector<double>> &columns)
{
    m_sourceData = columns;
    
    // 将数据列注册到JS引擎
    QJSValue dataObj = m_jsEngine->newObject();
    for (auto it = columns.begin(); it != columns.end(); ++it) {
        dataObj.setProperty(it.key(), vectorToJSArray(it.value()));
    }
    m_jsEngine->globalObject().setProperty("data", dataObj);
    
    // 同时添加派生列
    for (const auto &derived : m_derivedColumns) {
        dataObj.setProperty(derived.name, vectorToJSArray(derived.data));
    }
}

bool ScriptEngine::executeScript(const QString &script, const QString &outputColumnName)
{
    m_lastError.clear();
    
    if (outputColumnName.trimmed().isEmpty()) {
        m_lastError = "输出列名不能为空";
        return false;
    }
    
    // 检查是否与源数据列重名
    if (m_sourceData.contains(outputColumnName)) {
        m_lastError = QString("列名 \"%1\" 与源数据列重名").arg(outputColumnName);
        return false;
    }
    
    // 执行脚本
    QString wrappedScript = QString("(function() { %1 })()").arg(script);
    QJSValue result = m_jsEngine->evaluate(wrappedScript);
    
    if (result.isError()) {
        m_lastError = QString("脚本错误 (行 %1): %2")
            .arg(result.property("lineNumber").toInt())
            .arg(result.toString());
        return false;
    }
    
    // 检查返回值是否为数组
    if (!result.isArray()) {
        m_lastError = "脚本必须返回一个数组";
        return false;
    }
    
    // 转换结果
    QVector<double> resultData = jsArrayToVector(result);
    
    if (resultData.isEmpty()) {
        m_lastError = "脚本返回了空数组";
        return false;
    }
    
    // 保存派生列
    DerivedColumn derived;
    derived.name = outputColumnName;
    derived.data = resultData;
    derived.sourceScript = script;
    
    // 检查是否已存在同名派生列，如果是则替换
    for (int i = 0; i < m_derivedColumns.size(); ++i) {
        if (m_derivedColumns[i].name == outputColumnName) {
            m_derivedColumns[i] = derived;
            return true;
        }
    }
    
    m_derivedColumns.append(derived);
    return true;
}

QVector<double> ScriptEngine::getDerivedColumnData(const QString &name) const
{
    for (const auto &col : m_derivedColumns) {
        if (col.name == name) {
            return col.data;
        }
    }
    return QVector<double>();
}

void ScriptEngine::clearDerivedColumns()
{
    m_derivedColumns.clear();
}

bool ScriptEngine::removeDerivedColumn(const QString &name)
{
    for (int i = 0; i < m_derivedColumns.size(); ++i) {
        if (m_derivedColumns[i].name == name) {
            m_derivedColumns.removeAt(i);
            return true;
        }
    }
    return false;
}

QStringList ScriptEngine::getAllColumnNames() const
{
    QStringList names = m_sourceData.keys();
    for (const auto &col : m_derivedColumns) {
        names.append(col.name);
    }
    return names;
}

QString ScriptEngine::getHelpDocument()
{
    return R"(
=== LogParser 脚本引擎帮助 ===

【数据访问】
  data.列名          - 访问CSV中的列数据，返回数组
  例: data.Temperature, data.Pressure

【基本运算】
  math.add(a, b)      - 数组加法
  math.subtract(a, b) - 数组减法
  math.multiply(a, b) - 数组乘法
  math.divide(a, b)   - 数组除法
  math.scale(a, k)    - 数组乘以常数k
  math.offset(a, k)   - 数组加上常数k

【统计函数】
  math.sum(a)         - 求和
  math.mean(a)        - 平均值
  math.std_dev(a)     - 标准差
  math.variance(a)    - 方差
  math.min_val(a)     - 最小值
  math.max_val(a)     - 最大值
  math.rms(a)         - 均方根值

【归一化】
  math.normalize(a)              - 0-1归一化
  math.standardize(a)            - Z-score标准化
  math.normalize_range(a, min, max) - 范围归一化

【滤波器】
  math.moving_average(a, windowSize) - 移动平均
  math.median_filter(a, windowSize)  - 中值滤波
  math.lowpass_filter(a, cutoff)     - 低通滤波 (cutoff: 0-1)
  math.highpass_filter(a, cutoff)    - 高通滤波 (cutoff: 0-1)

【微积分】
  math.derivative(a, dt)  - 微分 (dt: 采样间隔，默认1)
  math.integral(a, dt)    - 积分
  math.cumsum(a)          - 累积和

【傅里叶变换】
  math.fft_magnitude(a)        - FFT幅值谱
  math.fft_phase(a)            - FFT相位谱
  math.fft_frequency(n, fs)    - 频率轴 (n: 点数, fs: 采样率)
  math.power_spectrum(a)       - 功率谱

【数学函数】
  math.abs_array(a)     - 绝对值
  math.sqrt_array(a)    - 平方根
  math.pow_array(a, n)  - 幂运算
  math.log_array(a)     - 自然对数
  math.log10_array(a)   - 常用对数
  math.exp_array(a)     - 指数
  math.sin_array(a)     - 正弦
  math.cos_array(a)     - 余弦

【信号生成】
  math.linspace(start, end, n)  - 等差数列
  math.zeros(n)                 - 全零数组
  math.ones(n)                  - 全一数组
  math.sine_wave(n, freq, amp, phase) - 正弦波

【相关性】
  math.cross_correlation(a, b)      - 互相关
  math.correlation_coefficient(a, b) - 相关系数

【插值】
  math.resample(a, newSize)          - 重采样
  math.interp_linear(x, y, xNew)     - 线性插值

【时间转换】
  math.ms_to_s_from_zero(a)    - 毫秒转秒，从0开始
  math.us_to_s_from_zero(a)    - 微秒转秒，从0开始
  math.ns_to_s_from_zero(a)    - 纳秒转秒，从0开始
  math.time_from_zero(a, scale) - 通用时间归零 (先减起始值，再乘以scale)

【示例脚本】

// 示例1: 两列相加
return math.add(data.Temperature, data.Pressure);

// 示例2: 归一化温度
return math.normalize(data.Temperature);

// 示例3: 计算速度的FFT幅值谱
return math.fft_magnitude(data.Speed);

// 示例4: 低通滤波后求导
var filtered = math.lowpass_filter(data.Voltage, 0.1);
return math.derivative(filtered, 0.01);

// 示例5: 计算两信号的相关系数并生成常数列
var corr = math.correlation_coefficient(data.Temperature, data.Humidity);
return math.scale(math.ones(data.Temperature.length), corr);

// 示例6: 将毫秒时间戳转换为从0开始的秒
return math.ms_to_s_from_zero(data.Timestamp);

// 示例7: 自定义时间转换（如微秒转毫秒，从0开始）
return math.time_from_zero(data.Timestamp, 0.001);
)";
}

void ScriptEngine::registerBuiltinFunctions()
{
    // 注册数学工具对象
    MathUtils *mathUtils = new MathUtils(this);
    QJSValue mathObj = m_jsEngine->newQObject(mathUtils);
    m_jsEngine->globalObject().setProperty("math", mathObj);
    
    // 添加一些常用的全局函数
    m_jsEngine->evaluate(R"(
        function len(arr) { return arr.length; }
        function print(msg) { console.log(msg); }
    )");
}

QJSValue ScriptEngine::vectorToJSArray(const QVector<double> &vec)
{
    QJSValue arr = m_jsEngine->newArray(vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        arr.setProperty(i, vec[i]);
    }
    return arr;
}

QVector<double> ScriptEngine::jsArrayToVector(const QJSValue &array)
{
    QVector<double> result;
    int length = array.property("length").toInt();
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        result.append(array.property(i).toNumber());
    }
    return result;
}

// ==================== MathUtils ====================

QVector<double> MathUtils::toVector(const QVariantList &list)
{
    QVector<double> result;
    result.reserve(list.size());
    for (const auto &v : list) {
        result.append(v.toDouble());
    }
    return result;
}

QVariantList MathUtils::toVariantList(const QVector<double> &vec)
{
    QVariantList result;
    for (double v : vec) {
        result.append(v);
    }
    return result;
}

// ===== 基本数组操作 =====

QVariantList MathUtils::add(const QVariantList &a, const QVariantList &b)
{
    QVector<double> va = toVector(a);
    QVector<double> vb = toVector(b);
    QVector<double> result(qMin(va.size(), vb.size()));
    for (int i = 0; i < result.size(); ++i) {
        result[i] = va[i] + vb[i];
    }
    return toVariantList(result);
}

QVariantList MathUtils::subtract(const QVariantList &a, const QVariantList &b)
{
    QVector<double> va = toVector(a);
    QVector<double> vb = toVector(b);
    QVector<double> result(qMin(va.size(), vb.size()));
    for (int i = 0; i < result.size(); ++i) {
        result[i] = va[i] - vb[i];
    }
    return toVariantList(result);
}

QVariantList MathUtils::multiply(const QVariantList &a, const QVariantList &b)
{
    QVector<double> va = toVector(a);
    QVector<double> vb = toVector(b);
    QVector<double> result(qMin(va.size(), vb.size()));
    for (int i = 0; i < result.size(); ++i) {
        result[i] = va[i] * vb[i];
    }
    return toVariantList(result);
}

QVariantList MathUtils::divide(const QVariantList &a, const QVariantList &b)
{
    QVector<double> va = toVector(a);
    QVector<double> vb = toVector(b);
    QVector<double> result(qMin(va.size(), vb.size()));
    for (int i = 0; i < result.size(); ++i) {
        result[i] = (vb[i] != 0) ? va[i] / vb[i] : 0;
    }
    return toVariantList(result);
}

QVariantList MathUtils::scale(const QVariantList &a, double factor)
{
    QVector<double> va = toVector(a);
    for (double &v : va) {
        v *= factor;
    }
    return toVariantList(va);
}

QVariantList MathUtils::offset(const QVariantList &a, double value)
{
    QVector<double> va = toVector(a);
    for (double &v : va) {
        v += value;
    }
    return toVariantList(va);
}

// ===== 统计函数 =====

double MathUtils::sum(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    return std::accumulate(va.begin(), va.end(), 0.0);
}

double MathUtils::mean(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return 0;
    return std::accumulate(va.begin(), va.end(), 0.0) / va.size();
}

double MathUtils::std_dev(const QVariantList &a)
{
    return std::sqrt(variance(a));
}

double MathUtils::variance(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.size() < 2) return 0;
    double m = mean(a);
    double sum = 0;
    for (double v : va) {
        sum += (v - m) * (v - m);
    }
    return sum / (va.size() - 1);
}

double MathUtils::min_val(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return 0;
    return *std::min_element(va.begin(), va.end());
}

double MathUtils::max_val(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return 0;
    return *std::max_element(va.begin(), va.end());
}

double MathUtils::rms(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return 0;
    double sumSq = 0;
    for (double v : va) {
        sumSq += v * v;
    }
    return std::sqrt(sumSq / va.size());
}

// ===== 归一化 =====

QVariantList MathUtils::normalize(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return a;
    
    double minV = *std::min_element(va.begin(), va.end());
    double maxV = *std::max_element(va.begin(), va.end());
    double range = maxV - minV;
    
    if (range == 0) {
        std::fill(va.begin(), va.end(), 0.5);
    } else {
        for (double &v : va) {
            v = (v - minV) / range;
        }
    }
    return toVariantList(va);
}

QVariantList MathUtils::standardize(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    if (va.size() < 2) return a;
    
    double m = mean(a);
    double s = std_dev(a);
    
    if (s == 0) {
        std::fill(va.begin(), va.end(), 0);
    } else {
        for (double &v : va) {
            v = (v - m) / s;
        }
    }
    return toVariantList(va);
}

QVariantList MathUtils::normalize_range(const QVariantList &a, double newMin, double newMax)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return a;
    
    double minV = *std::min_element(va.begin(), va.end());
    double maxV = *std::max_element(va.begin(), va.end());
    double range = maxV - minV;
    
    if (range == 0) {
        std::fill(va.begin(), va.end(), (newMin + newMax) / 2);
    } else {
        for (double &v : va) {
            v = newMin + (v - minV) / range * (newMax - newMin);
        }
    }
    return toVariantList(va);
}

// ===== 滤波器 =====

QVariantList MathUtils::moving_average(const QVariantList &a, int windowSize)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty() || windowSize < 1) return a;
    
    windowSize = qMin(windowSize, va.size());
    QVector<double> result(va.size());
    
    for (int i = 0; i < va.size(); ++i) {
        int start = qMax(0, i - windowSize / 2);
        int end = qMin(va.size(), i + windowSize / 2 + 1);
        double sum = 0;
        for (int j = start; j < end; ++j) {
            sum += va[j];
        }
        result[i] = sum / (end - start);
    }
    return toVariantList(result);
}

QVariantList MathUtils::median_filter(const QVariantList &a, int windowSize)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty() || windowSize < 1) return a;
    
    windowSize = qMin(windowSize, va.size());
    QVector<double> result(va.size());
    
    for (int i = 0; i < va.size(); ++i) {
        int start = qMax(0, i - windowSize / 2);
        int end = qMin(va.size(), i + windowSize / 2 + 1);
        QVector<double> window;
        for (int j = start; j < end; ++j) {
            window.append(va[j]);
        }
        std::sort(window.begin(), window.end());
        result[i] = window[window.size() / 2];
    }
    return toVariantList(result);
}

QVariantList MathUtils::lowpass_filter(const QVariantList &a, double cutoffRatio)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return a;
    
    cutoffRatio = qBound(0.0, cutoffRatio, 1.0);
    double alpha = cutoffRatio;
    
    QVector<double> result(va.size());
    result[0] = va[0];
    for (int i = 1; i < va.size(); ++i) {
        result[i] = alpha * va[i] + (1 - alpha) * result[i - 1];
    }
    return toVariantList(result);
}

QVariantList MathUtils::highpass_filter(const QVariantList &a, double cutoffRatio)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return a;
    
    QVariantList lowpassed = lowpass_filter(a, cutoffRatio);
    return subtract(a, lowpassed);
}

// ===== 微积分 =====

QVariantList MathUtils::derivative(const QVariantList &a, double dt)
{
    QVector<double> va = toVector(a);
    if (va.size() < 2) return a;
    
    QVector<double> result(va.size());
    result[0] = (va[1] - va[0]) / dt;
    for (int i = 1; i < va.size() - 1; ++i) {
        result[i] = (va[i + 1] - va[i - 1]) / (2 * dt);
    }
    result[va.size() - 1] = (va[va.size() - 1] - va[va.size() - 2]) / dt;
    return toVariantList(result);
}

QVariantList MathUtils::integral(const QVariantList &a, double dt)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return a;
    
    QVector<double> result(va.size());
    result[0] = 0;
    for (int i = 1; i < va.size(); ++i) {
        result[i] = result[i - 1] + (va[i] + va[i - 1]) / 2 * dt;
    }
    return toVariantList(result);
}

QVariantList MathUtils::cumsum(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    QVector<double> result(va.size());
    double sum = 0;
    for (int i = 0; i < va.size(); ++i) {
        sum += va[i];
        result[i] = sum;
    }
    return toVariantList(result);
}

// ===== 傅里叶变换 =====

void MathUtils::fft_impl(std::vector<std::complex<double>> &data, bool inverse)
{
    int n = data.size();
    if (n <= 1) return;
    
    // 位反转置换
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }
    
    // Cooley-Tukey FFT
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2 * M_PI / len * (inverse ? 1 : -1);
        std::complex<double> wn(cos(angle), sin(angle));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = data[i + j];
                std::complex<double> t = w * data[i + j + len / 2];
                data[i + j] = u + t;
                data[i + j + len / 2] = u - t;
                w *= wn;
            }
        }
    }
    
    if (inverse) {
        for (auto &x : data) {
            x /= n;
        }
    }
}

QVariantList MathUtils::fft_magnitude(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    
    // 补零到2的幂次
    int n = 1;
    while (n < va.size()) n <<= 1;
    
    std::vector<std::complex<double>> data(n);
    for (int i = 0; i < va.size(); ++i) {
        data[i] = std::complex<double>(va[i], 0);
    }
    
    fft_impl(data, false);
    
    // 只返回正频率部分
    QVector<double> result(n / 2 + 1);
    for (int i = 0; i <= n / 2; ++i) {
        result[i] = std::abs(data[i]) * 2 / n;
    }
    result[0] /= 2;  // DC分量不需要乘2
    
    return toVariantList(result);
}

QVariantList MathUtils::fft_phase(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    
    int n = 1;
    while (n < va.size()) n <<= 1;
    
    std::vector<std::complex<double>> data(n);
    for (int i = 0; i < va.size(); ++i) {
        data[i] = std::complex<double>(va[i], 0);
    }
    
    fft_impl(data, false);
    
    QVector<double> result(n / 2 + 1);
    for (int i = 0; i <= n / 2; ++i) {
        result[i] = std::arg(data[i]);
    }
    
    return toVariantList(result);
}

QVariantList MathUtils::fft_frequency(int n, double sampleRate)
{
    int fftSize = 1;
    while (fftSize < n) fftSize <<= 1;
    
    QVector<double> result(fftSize / 2 + 1);
    for (int i = 0; i <= fftSize / 2; ++i) {
        result[i] = i * sampleRate / fftSize;
    }
    return toVariantList(result);
}

QVariantList MathUtils::power_spectrum(const QVariantList &a)
{
    QVariantList mag = fft_magnitude(a);
    QVector<double> vm = toVector(mag);
    for (double &v : vm) {
        v = v * v;
    }
    return toVariantList(vm);
}

// ===== 数学函数 =====

QVariantList MathUtils::abs_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::abs(v);
    return toVariantList(va);
}

QVariantList MathUtils::sqrt_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::sqrt(qMax(0.0, v));
    return toVariantList(va);
}

QVariantList MathUtils::pow_array(const QVariantList &a, double exp)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::pow(v, exp);
    return toVariantList(va);
}

QVariantList MathUtils::log_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = (v > 0) ? std::log(v) : 0;
    return toVariantList(va);
}

QVariantList MathUtils::log10_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = (v > 0) ? std::log10(v) : 0;
    return toVariantList(va);
}

QVariantList MathUtils::exp_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::exp(v);
    return toVariantList(va);
}

QVariantList MathUtils::sin_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::sin(v);
    return toVariantList(va);
}

QVariantList MathUtils::cos_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::cos(v);
    return toVariantList(va);
}

QVariantList MathUtils::tan_array(const QVariantList &a)
{
    QVector<double> va = toVector(a);
    for (double &v : va) v = std::tan(v);
    return toVariantList(va);
}

// ===== 信号生成 =====

QVariantList MathUtils::linspace(double start, double end, int n)
{
    QVector<double> result(n);
    if (n == 1) {
        result[0] = start;
    } else {
        double step = (end - start) / (n - 1);
        for (int i = 0; i < n; ++i) {
            result[i] = start + i * step;
        }
    }
    return toVariantList(result);
}

QVariantList MathUtils::zeros(int n)
{
    return toVariantList(QVector<double>(n, 0));
}

QVariantList MathUtils::ones(int n)
{
    return toVariantList(QVector<double>(n, 1));
}

QVariantList MathUtils::sine_wave(int n, double frequency, double amplitude, double phase)
{
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = amplitude * std::sin(2 * M_PI * frequency * i / n + phase);
    }
    return toVariantList(result);
}

// ===== 相关性分析 =====

QVariantList MathUtils::cross_correlation(const QVariantList &a, const QVariantList &b)
{
    QVector<double> va = toVector(a);
    QVector<double> vb = toVector(b);
    int n = qMax(va.size(), vb.size());
    
    QVector<double> result(2 * n - 1, 0);
    
    for (int lag = -(n - 1); lag < n; ++lag) {
        double sum = 0;
        int count = 0;
        for (int i = 0; i < va.size(); ++i) {
            int j = i + lag;
            if (j >= 0 && j < vb.size()) {
                sum += va[i] * vb[j];
                count++;
            }
        }
        result[lag + n - 1] = (count > 0) ? sum / count : 0;
    }
    return toVariantList(result);
}

double MathUtils::correlation_coefficient(const QVariantList &a, const QVariantList &b)
{
    QVector<double> va = toVector(a);
    QVector<double> vb = toVector(b);
    int n = qMin(va.size(), vb.size());
    if (n < 2) return 0;
    
    double meanA = 0, meanB = 0;
    for (int i = 0; i < n; ++i) {
        meanA += va[i];
        meanB += vb[i];
    }
    meanA /= n;
    meanB /= n;
    
    double cov = 0, varA = 0, varB = 0;
    for (int i = 0; i < n; ++i) {
        double da = va[i] - meanA;
        double db = vb[i] - meanB;
        cov += da * db;
        varA += da * da;
        varB += db * db;
    }
    
    if (varA == 0 || varB == 0) return 0;
    return cov / std::sqrt(varA * varB);
}

// ===== 插值与重采样 =====

QVariantList MathUtils::resample(const QVariantList &a, int newSize)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty() || newSize < 1) return a;
    
    QVector<double> result(newSize);
    double ratio = (double)(va.size() - 1) / (newSize - 1);
    
    for (int i = 0; i < newSize; ++i) {
        double idx = i * ratio;
        int idx0 = (int)idx;
        int idx1 = qMin(idx0 + 1, va.size() - 1);
        double frac = idx - idx0;
        result[i] = va[idx0] * (1 - frac) + va[idx1] * frac;
    }
    return toVariantList(result);
}

QVariantList MathUtils::interp_linear(const QVariantList &x, const QVariantList &y, const QVariantList &xNew)
{
    QVector<double> vx = toVector(x);
    QVector<double> vy = toVector(y);
    QVector<double> vxNew = toVector(xNew);
    
    if (vx.size() != vy.size() || vx.isEmpty()) {
        return xNew;
    }
    
    QVector<double> result(vxNew.size());
    
    for (int i = 0; i < vxNew.size(); ++i) {
        double xi = vxNew[i];
        
        // 找到插值区间
        int j = 0;
        while (j < vx.size() - 1 && vx[j + 1] < xi) j++;
        
        if (j >= vx.size() - 1) {
            result[i] = vy.last();
        } else if (xi <= vx[0]) {
            result[i] = vy[0];
        } else {
            double t = (xi - vx[j]) / (vx[j + 1] - vx[j]);
            result[i] = vy[j] * (1 - t) + vy[j + 1] * t;
        }
    }
    return toVariantList(result);
}

// ==================== 时间转换函数 ====================

QVariantList MathUtils::ms_to_s_from_zero(const QVariantList &a)
{
    // 毫秒转秒，从0开始
    return time_from_zero(a, 0.001);
}

QVariantList MathUtils::us_to_s_from_zero(const QVariantList &a)
{
    // 微秒转秒，从0开始
    return time_from_zero(a, 0.000001);
}

QVariantList MathUtils::ns_to_s_from_zero(const QVariantList &a)
{
    // 纳秒转秒，从0开始
    return time_from_zero(a, 0.000000001);
}

QVariantList MathUtils::time_from_zero(const QVariantList &a, double scaleFactor)
{
    QVector<double> va = toVector(a);
    if (va.isEmpty()) return a;
    
    // 获取起始时间（第一个值）
    double startTime = va[0];
    
    // 减去起始时间并乘以缩放因子
    QVector<double> result(va.size());
    for (int i = 0; i < va.size(); ++i) {
        result[i] = (va[i] - startTime) * scaleFactor;
    }
    
    return toVariantList(result);
}
