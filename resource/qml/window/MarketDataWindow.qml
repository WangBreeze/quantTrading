import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// 市场数据窗口，可以作为独立窗口或嵌入到主应用程序中
Window {
    id: root
    width: 800
    height: 600
    title: "市场数据分析"
    visible: true
    
    // 属性声明
    property var marketDataModel: null  // 市场数据模型
    
    // 窗口内容
    MarketDataPage {
        id: marketDataPage
        anchors.fill: parent
        marketDataModel: root.marketDataModel
    }
    
    // 设置数据模型
    function setModel(model) {
        marketDataModel = model;
        marketDataPage.setModel(model);
    }
    
    // 添加测试数据的函数（仅用于演示）
    function addTestData() {
        if (!marketDataModel) return;
        
        // 创建一些测试数据
        var now = new Date();
        var baseTimestamp = now.getTime() - (24 * 60 * 60 * 1000); // 从24小时前开始
        var basePrice = 45000; // 基础价格
        
        // 清除现有数据
        marketDataModel.clearData();
        
        // 添加48个小时的数据点（每30分钟一个）
        for (var i = 0; i < 48 * 2; i++) {
            var timestamp = new Date(baseTimestamp + i * 30 * 60 * 1000);
            
            // 创建一些随机波动
            var randomFactor = 1 + (Math.random() * 0.02 - 0.01); // -1% 到 +1% 的随机波动
            var price = basePrice * randomFactor;
            
            // 创建开高低收价格
            var open = price;
            var close = price * (1 + (Math.random() * 0.01 - 0.005)); // -0.5% 到 +0.5% 的随机波动
            var high = Math.max(open, close) * (1 + Math.random() * 0.005); // 最高价比开盘价或收盘价高0-0.5%
            var low = Math.min(open, close) * (1 - Math.random() * 0.005); // 最低价比开盘价或收盘价低0-0.5%
            
            // 创建成交量
            var volume = Math.random() * 10 + 1; // 1-11 的随机成交量
            
            // 创建 MarketData 对象
            var marketData = {
                "symbol": "BTC/USDT",
                "timestamp": timestamp,
                "price": close,
                "open": open,
                "high": high,
                "low": low,
                "close": close,
                "volume": volume,
                "amount": volume * close,
                "tickCount": Math.floor(Math.random() * 100) + 10,
                "bidPrice": close * 0.999,
                "askPrice": close * 1.001,
                "bidVolume": Math.random() * 5,
                "askVolume": Math.random() * 5
            };
            
            // 更新基础价格为当前收盘价
            basePrice = close;
            
            // 添加到模型
            marketDataModel.onNewMarketData(marketData);
        }
    }
    
    // 初始化函数
    Component.onCompleted: {
        // 如果有模型，则设置
        if (marketDataModel) {
            marketDataPage.setModel(marketDataModel);
        }
    }
}