#ifndef ONLINEMARKET_A_H
#define ONLINEMARKET_A_H

#include "onlinemarket.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QUrl>
#include <QtWebSockets/QWebSocket>
#include <QDateTime>
#include <QMap>

class onLineMarket_A : public onLineMarket
{
    Q_OBJECT
public:
    explicit onLineMarket_A(QObject *parent = nullptr);
    ~onLineMarket_A();

    // 初始化网络连接和定时器
    void initialize();
    
    // 设置要获取的股票代码
    void setSymbol(const QString &symbol) override;
    
    // 设置数据获取间隔（毫秒）- 仅用于REST API模式
    void setUpdateInterval(int milliseconds) override;
    
    // 设置数据获取模式（WebSocket或REST API）
    enum DataFetchMode { REST_API, WEBSOCKET };
    void setDataFetchMode(DataFetchMode mode);
    
    // 开始获取行情数据
    void startFetchingData() override;
    
    // 停止获取行情数据
    void stopFetchingData() override;

private slots:
    // REST API 相关槽函数
    void onNetworkReplyFinished();
    void fetchMarketData();
    
    // WebSocket 相关槽函数
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString &message);
    void onWebSocketError(QAbstractSocket::SocketError error);
    
    // 心跳定时器
    void sendHeartbeat();

private:
    // 解析JSON响应
    void parseMarketData(const QByteArray &data);
    void parseWebSocketMessage(const QString &message);
    
    // 初始化WebSocket连接
    void initializeWebSocket();
    
    // 订阅WebSocket频道
    void subscribeToChannel();
    
    // 获取股票市场类型（上海或深圳）
    QString getMarketType(const QString &symbol);
    
    // 格式化股票代码（添加市场前缀）
    QString formatStockCode(const QString &symbol);

    // REST API相关成员
    QNetworkAccessManager *m_networkManager;
    QTimer *m_dataFetchTimer;
    QUrl m_apiUrl;
    
    // WebSocket相关成员
    QWebSocket *m_webSocket;
    bool m_webSocketConnected;
    QTimer *m_heartbeatTimer;
    
    // 通用成员
    QString m_symbol;           // 股票代码
    QString m_formattedSymbol;  // 格式化后的股票代码（带市场前缀）
    bool m_isActive;
    DataFetchMode m_dataFetchMode;
    
    // 东方财富API相关常量
    const QString API_BASE_URL = "https://push2.eastmoney.com";
    const QString API_QUOTE_ENDPOINT = "/api/qt/stock/get";
    const QString WS_BASE_URL = "wss://wss.dandanzhuan.com/ws";
    
    // 股票行情数据缓存
    QMap<QString, double> m_lastData;
};

#endif // ONLINEMARKET_A_H
