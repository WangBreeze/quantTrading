import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

// 市场数据页面，展示如何使用 MarketDataChartView
Page {
    id: root
    
    // 属性声明
    property var marketDataModel: null  // 市场数据模型
    property string currentSymbol: "BTC/USDT"  // 当前交易品种
    
    // 页面布局
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10
        
        // 标题和控制区域
        RowLayout {
            Layout.fillWidth: true
            
            Label {
                text: "市场数据"
                font.pixelSize: 20
                font.bold: true
            }
            
            Item { Layout.fillWidth: true }  // 弹性空间
            
            // 交易品种选择
            ComboBox {
                id: symbolSelector
                model: ["BTC/USDT", "ETH/USDT", "BNB/USDT", "XRP/USDT"]
                onCurrentTextChanged: {
                    currentSymbol = currentText;
                    marketDataChart.setSymbol(currentSymbol);
                }
            }
            
            // 刷新按钮
            Button {
                text: "刷新"
                onClicked: {
                    if (marketDataModel) {
                        marketDataModel.updateData();
                    }
                }
            }
        }
        
        // 市场数据图表
        MarketDataChartView {
            id: marketDataChart
            Layout.fillWidth: true
            Layout.fillHeight: true
            marketDataModel: root.marketDataModel
            symbol: currentSymbol
            
            // 处理图表点击事件
            onChartClicked: function(point) {
                console.log("图表点击位置:", point.x, point.y);
            }
            
            // 处理周期变更事件
            onPeriodChanged: function(period) {
                console.log("周期变更为:", period);
                // 这里可以添加切换不同周期数据的逻辑
            }
        }
        
        // 底部状态栏
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "#f0f0f0"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10
                
                Label {
                    text: "最新价: " + (marketDataModel && marketDataModel.rowCount() > 0 ? 
                           marketDataModel.data(marketDataModel.index(marketDataModel.rowCount() - 1, 0)).close.toFixed(2) : "0.00")
                    font.bold: true
                }
                
                Label {
                    text: "成交量: " + (marketDataModel && marketDataModel.rowCount() > 0 ? 
                           marketDataModel.data(marketDataModel.index(marketDataModel.rowCount() - 1, 0)).volume.toFixed(2) : "0.00")
                }
                
                Item { Layout.fillWidth: true }  // 弹性空间
                
                Label {
                    text: "更新时间: " + Qt.formatDateTime(new Date(), "yyyy-MM-dd hh:mm:ss")
                }
            }
        }
    }
    
    // 初始化函数
    Component.onCompleted: {
        if (marketDataModel) {
            marketDataChart.setModel(marketDataModel);
        }
    }
    
    // 设置数据模型
    function setModel(model) {
        marketDataModel = model;
        if (marketDataChart) {
            marketDataChart.setModel(model);
        }
    }
}