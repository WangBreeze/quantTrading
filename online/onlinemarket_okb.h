#ifndef ONLINEMARKET_OKB_H
#define ONLINEMARKET_OKB_H

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

class onLineMarket_OKB : public onLineMarket
{
    Q_OBJECT
public:
    explicit onLineMarket_OKB(QObject *parent = nullptr);
    ~onLineMarket_OKB();

    // 初始化网络连接和定时器
    void initialize();
    
    // 设置要获取的交易对
    void setSymbol(const QString &symbol);
    
    // 设置数据获取间隔（毫秒）- 仅用于REST API模式
    void setUpdateInterval(int milliseconds);
    
    // 设置数据获取模式（WebSocket或REST API）
    enum DataFetchMode { REST_API, WEBSOCKET };
    void setDataFetchMode(DataFetchMode mode);
    
    // 开始获取行情数据
    void startFetchingData();
    
    // 停止获取行情数据
    void stopFetchingData();

signals:
    // 当收到新的行情数据时发出信号
    void newMarketData(const AppData::MarketData &data);
    
    // 错误信号
    void errorOccurred(const QString &errorMessage);

private slots:
    // REST API 相关槽函数
    void onNetworkReplyFinished();
    void fetchMarketData();
    
    // WebSocket 相关槽函数
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString &message);
    void onWebSocketError(QAbstractSocket::SocketError error);

private:
    // 解析JSON响应
    void parseMarketData(const QByteArray &data);
    void parseWebSocketMessage(const QString &message);
    
    // 创建MarketData对象
    AppData::MarketData createMarketData(const QString &symbol, double price, 
                                       double volume, double high, double low,
                                       double open, double close);
    
    // 初始化WebSocket连接
    void initializeWebSocket();
    
    // 订阅WebSocket频道
    void subscribeToChannel();

    // REST API相关成员
    QNetworkAccessManager *m_networkManager;
    QTimer *m_dataFetchTimer;
    QUrl m_apiUrl;
    
    // WebSocket相关成员
    QWebSocket *m_webSocket;
    bool m_webSocketConnected;
    
    // 通用成员
    QString m_symbol;
    bool m_isActive;
    DataFetchMode m_dataFetchMode;
    
    // 欧易API相关常量
    const QString API_BASE_URL = "https://www.okx.com";
    const QString API_MARKET_ENDPOINT = "/api/v5/market/ticker";
    const QString WS_BASE_URL = "wss://ws.okx.com:8443/ws/v5/public";
};

#endif // ONLINEMARKET_OKB_H
