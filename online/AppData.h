#ifndef APPDATA_H
#define APPDATA_H

#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QVariant>

namespace AppData {

// 交易方向枚举
enum Direction {
    Long = 0,       // 做多
    Short = 1,      // 做空
    Unknown = 2     // 未知
};

// 订单类型枚举
enum OrderType {
    Market = 0,     // 市价单
    Limit = 1,      // 限价单
    Stop = 2,       // 止损单
    StopLimit = 3,  // 止损限价单
    TrailingStop = 4 // 追踪止损单
};

// 订单状态枚举
enum OrderStatus {
    Created = 0,    // 已创建
    Submitted = 1,  // 已提交
    Accepted = 2,   // 已接受
    Partial = 3,    // 部分成交
    Completed = 4,  // 全部成交
    Canceled = 5,   // 已取消
    Rejected = 6,   // 已拒绝
    Expired = 7     // 已过期
};

// 交易所类型枚举
enum ExchangeType {
    SSE = 0,        // 上海证券交易所
    SZSE = 1,       // 深圳证券交易所
    CFFEX = 2,      // 中国金融期货交易所
    SHFE = 3,       // 上海期货交易所
    DCE = 4,        // 大连商品交易所
    CZCE = 5,       // 郑州商品交易所
    INE = 6,        // 上海国际能源交易中心
    BSE = 7,        // 北京证券交易所
    OKX = 8,        // 欧易交易所
    Binance = 9,    // 币安交易所
    Other = 10      // 其他交易所
};

// 产品类型枚举
enum ProductType {
    Stock = 0,      // 股票
    Future = 1,     // 期货
    Option = 2,     // 期权
    Bond = 3,       // 债券
    Forex = 4,      // 外汇
    Crypto = 5,     // 加密货币
    Index = 6,      // 指数
    ETF = 7,        // ETF
    Fund = 8,       // 基金
    Other = 9       // 其他
};

// 时间周期枚举
enum TimeFrame {
    Tick = 0,       // Tick数据
    Second = 1,     // 秒线
    Minute = 2,     // 分钟线
    Hour = 3,       // 小时线
    Day = 4,        // 日线
    Week = 5,       // 周线
    Month = 6,      // 月线
    Quarter = 7,    // 季线
    Year = 8        // 年线
};

// 订单数据结构
struct Order {
    QString orderId;            // 订单ID
    QString symbol;             // 交易品种代码
    QDateTime createTime;       // 创建时间
    QDateTime updateTime;       // 更新时间
    Direction direction;        // 交易方向
    OrderType type;             // 订单类型
    OrderStatus status;         // 订单状态
    double price;               // 价格
    double stopPrice;           // 止损价格（用于止损单）
    double quantity;            // 数量
    double filledQuantity;      // 已成交数量
    double avgFillPrice;        // 平均成交价格
    double commission;          // 手续费
    QString exchangeOrderId;    // 交易所订单ID
    QString accountId;          // 账户ID
    QString remark;             // 备注
    QMap<QString, QVariant> extraInfo; // 额外信息

    Order() : direction(Unknown), type(Market), status(Created),
              price(0.0), stopPrice(0.0), quantity(0.0),
              filledQuantity(0.0), avgFillPrice(0.0), commission(0.0) {}
};

// 成交记录数据结构
struct Trade {
    QString tradeId;            // 成交ID
    QString orderId;            // 关联的订单ID
    QString symbol;             // 交易品种代码
    QDateTime tradeTime;        // 成交时间
    Direction direction;        // 交易方向
    double price;               // 成交价格
    double quantity;            // 成交数量
    double commission;          // 手续费
    QString exchangeTradeId;    // 交易所成交ID
    QString accountId;          // 账户ID
    QMap<QString, QVariant> extraInfo; // 额外信息

    Trade() : direction(Unknown), price(0.0), quantity(0.0), commission(0.0) {}
};

// 持仓数据结构
struct Position {
    QString symbol;             // 交易品种代码
    Direction direction;        // 持仓方向
    double quantity;            // 持仓数量
    double avgPrice;            // 平均持仓价格
    double marketPrice;         // 当前市场价格
    double unrealizedPnL;       // 未实现盈亏
    double realizedPnL;         // 已实现盈亏
    QDateTime openTime;         // 开仓时间
    QString accountId;          // 账户ID
    QMap<QString, QVariant> extraInfo; // 额外信息

    Position() : direction(Unknown), quantity(0.0), avgPrice(0.0),
                 marketPrice(0.0), unrealizedPnL(0.0), realizedPnL(0.0) {}
};

// 账户数据结构
struct Account {
    QString accountId;          // 账户ID
    QString name;               // 账户名称
    double balance;             // 账户余额
    double available;           // 可用资金
    double margin;              // 保证金
    double unrealizedPnL;       // 未实现盈亏
    double realizedPnL;         // 已实现盈亏
    QMap<QString, Position> positions; // 持仓信息
    QMap<QString, QVariant> extraInfo; // 额外信息

    Account() : balance(0.0), available(0.0), margin(0.0),
                unrealizedPnL(0.0), realizedPnL(0.0) {}
};

// 行情数据结构
struct MarketData {
    QString symbol;             // 交易品种代码
    QDateTime timestamp;        // 时间戳
    double open;                // 开盘价
    double high;                // 最高价
    double low;                 // 最低价
    double close;               // 收盘价
    double volume;              // 成交量
    double amount;              // 成交额
    int tickCount;              // 成交笔数
    double openInterest;        // 持仓量（期货）
    double bidPrice;            // 买一价
    double askPrice;            // 卖一价
    double bidVolume;           // 买一量
    double askVolume;           // 卖一量
    QVector<double> bidPrices;  // 买盘价格
    QVector<double> askPrices;  // 卖盘价格
    QVector<double> bidVolumes; // 买盘量
    QVector<double> askVolumes; // 卖盘量
    QMap<QString, QVariant> extraInfo; // 额外信息

    MarketData() : open(0.0), high(0.0), low(0.0), close(0.0),
                   volume(0.0), amount(0.0), tickCount(0),
                   openInterest(0.0), bidPrice(0.0), askPrice(0.0),
                   bidVolume(0.0), askVolume(0.0) {}
};

// K线数据结构
struct Candle {
    QString symbol;             // 交易品种代码
    TimeFrame timeFrame;        // 时间周期
    QDateTime timestamp;        // 时间戳
    double open;                // 开盘价
    double high;                // 最高价
    double low;                 // 最低价
    double close;               // 收盘价
    double volume;              // 成交量
    double amount;              // 成交额
    int tickCount;              // 成交笔数
    double openInterest;        // 持仓量（期货）
    QMap<QString, QVariant> extraInfo; // 额外信息

    Candle() : timeFrame(Minute), open(0.0), high(0.0), low(0.0), close(0.0),
               volume(0.0), amount(0.0), tickCount(0), openInterest(0.0) {}
};

// 回测参数结构
struct BacktestParams {
    QDateTime startDate;        // 回测开始日期
    QDateTime endDate;          // 回测结束日期
    QVector<QString> symbols;   // 回测品种列表
    TimeFrame timeFrame;        // 回测时间周期
    double initialCapital;      // 初始资金
    double commission;          // 手续费率
    double slippage;            // 滑点
    bool useAdjustedPrice;      // 是否使用复权价格
    QMap<QString, QVariant> extraParams; // 额外参数

    BacktestParams() : timeFrame(Day), initialCapital(1000000.0),
                       commission(0.0003), slippage(0.0), useAdjustedPrice(true) {}
};

// 回测结果结构
struct BacktestResult {
    double finalCapital;        // 最终资金
    double totalReturn;         // 总收益率
    double annualReturn;        // 年化收益率
    double sharpeRatio;         // 夏普比率
    double maxDrawdown;         // 最大回撤
    double winRate;             // 胜率
    int totalTrades;            // 总交易次数
    int winTrades;              // 盈利交易次数
    int lossTrades;             // 亏损交易次数
    double profitFactor;        // 盈亏比
    double averageProfit;       // 平均盈利
    double averageLoss;         // 平均亏损
    QVector<Trade> trades;      // 交易记录
    QVector<double> equityCurve; // 权益曲线
    QVector<QDateTime> equityTimes; // 权益曲线对应的时间点
    QMap<QString, QVariant> extraResults; // 额外结果

    BacktestResult() : finalCapital(0.0), totalReturn(0.0), annualReturn(0.0),
                       sharpeRatio(0.0), maxDrawdown(0.0), winRate(0.0),
                       totalTrades(0), winTrades(0), lossTrades(0),
                       profitFactor(0.0), averageProfit(0.0), averageLoss(0.0) {}
};

// 交易信号结构
struct Signal {
    QString symbol;             // 交易品种代码
    QDateTime timestamp;        // 信号时间
    Direction direction;        // 信号方向
    double price;               // 信号价格
    double quantity;            // 建议数量
    QString strategy;           // 策略名称
    int strength;               // 信号强度（1-10）
    QString message;            // 信号描述
    QMap<QString, QVariant> extraInfo; // 额外信息

    Signal() : direction(Unknown), price(0.0), quantity(0.0), strength(5) {}
};

// 交易品种信息结构
struct Instrument {
    QString symbol;             // 交易品种代码
    QString name;               // 交易品种名称
    ExchangeType exchange;      // 交易所
    ProductType productType;    // 产品类型
    double priceTick;           // 最小价格变动单位
    double contractSize;        // 合约规模
    QString currency;           // 货币
    QDateTime listDate;         // 上市日期
    QDateTime expireDate;       // 到期日期（期货/期权）
    double marginRate;          // 保证金率
    double commissionRate;      // 手续费率
    QMap<QString, QVariant> extraInfo; // 额外信息

    Instrument() : exchange(SSE), productType(Stock), priceTick(0.01),
                   contractSize(1.0), marginRate(0.0), commissionRate(0.0) {}
};

} // namespace AppData

#endif // APPDATA_H