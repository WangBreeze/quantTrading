import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtCharts 2.15

// 封装的市场数据图表视图组件
Item {
    id: root
    
    // 属性声明
    property var marketDataModel: null  // 市场数据模型
    property string symbol: ""          // 交易品种代码
    property string title: "市场数据图表"  // 图表标题
    property string dateFormat: "yyyy-MM-dd hh:mm:ss"  // 日期格式
    property color increasingColor: "green"  // 上涨颜色
    property color decreasingColor: "red"    // 下跌颜色
    property bool showVolume: true           // 是否显示成交量
    property bool autoScale: true            // 是否自动缩放
    
    // 信号声明
    signal chartClicked(var point)
    signal periodChanged(string period)
    
    // 布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 5
        
        // 工具栏
        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            
            Label {
                text: symbol
                font.bold: true
                font.pixelSize: 16
            }
            
            Item { Layout.fillWidth: true }  // 弹性空间
            
            // 周期选择按钮组
            Row {
                spacing: 5
                
                Button {
                    text: "1分钟"
                    onClicked: periodChanged("M1")
                }
                
                Button {
                    text: "5分钟"
                    onClicked: periodChanged("M5")
                }
                
                Button {
                    text: "15分钟"
                    onClicked: periodChanged("M15")
                }
                
                Button {
                    text: "1小时"
                    onClicked: periodChanged("H1")
                }
                
                Button {
                    text: "日线"
                    onClicked: periodChanged("D1")
                }
            }
        }
        
        // K线图表
        ChartView {
            id: chartView
            Layout.fillWidth: true
            Layout.fillHeight: true
            title: root.title
            antialiasing: true
            legend.visible: false
            
            // 日期时间轴
            DateTimeAxis {
                id: dateAxis
                format: root.dateFormat
                tickCount: 5
                labelsAngle: -45
            }
            
            // 价格轴
            ValueAxis {
                id: priceAxis
                labelFormat: "%.2f"
                tickCount: 5
            }
            
            // 成交量轴
            ValueAxis {
                id: volumeAxis
                visible: showVolume
                labelsVisible: false
                gridVisible: false
                min: 0
            }
            
            // K线图序列
            CandlestickSeries {
                id: candleSeries
                name: symbol
                increasingColor: root.increasingColor
                decreasingColor: root.decreasingColor
                axisX: dateAxis
                axisY: priceAxis
                
                // 点击事件处理
                onClicked: {
                    root.chartClicked(point);
                }
            }
            
            // 成交量序列
            BarSeries {
                id: volumeSeries
                visible: showVolume
                axisX: dateAxis
                axisYRight: volumeAxis
                barWidth: 0.5
            }
        }
    }
    
    // 初始化函数
    Component.onCompleted: {
        updateChart();
    }
    
    // 当模型数据变化时更新图表
    Connections {
        target: marketDataModel
        function onDataChanged() {
            updateChart();
        }
    }
    
    // 更新图表数据
    function updateChart() {
        if (!marketDataModel || marketDataModel.rowCount() === 0) {
            return;
        }
        
        // 清除现有数据
        candleSeries.clear();
        
        if (volumeSeries.count > 0) {
            volumeSeries.clear();
        }
        
        var volumeSet = volumeSeries.append("Volume", []);
        
        // 价格范围
        var minPrice = Number.MAX_VALUE;
        var maxPrice = Number.MIN_VALUE;
        var maxVolume = 0;
        
        // 时间范围
        var minDate = new Date();
        var maxDate = new Date(0);
        
        // 添加数据点
        for (var i = 0; i < marketDataModel.rowCount(); i++) {
            var data = marketDataModel.data(marketDataModel.index(i, 0));
            
            // 创建K线数据点
            var timestamp = data.timestamp.getTime();
            var candleSet = Qt.createQmlObject(
                'import QtCharts 2.15; CandlestickSet { timestamp: ' + timestamp + 
                '; open: ' + data.open + 
                '; high: ' + data.high + 
                '; low: ' + data.low + 
                '; close: ' + data.close + ' }',
                candleSeries
            );
            candleSeries.append(candleSet);
            
            // 添加成交量数据
            if (showVolume) {
                volumeSet.append(timestamp, data.volume);
            }
            
            // 更新范围
            minPrice = Math.min(minPrice, data.low);
            maxPrice = Math.max(maxPrice, data.high);
            maxVolume = Math.max(maxVolume, data.volume);
            
            var dataDate = data.timestamp;
            if (dataDate < minDate) minDate = dataDate;
            if (dataDate > maxDate) maxDate = dataDate;
        }
        
        // 设置轴范围
        if (autoScale && marketDataModel.rowCount() > 0) {
            // 为价格轴添加一些边距
            var pricePadding = (maxPrice - minPrice) * 0.05;
            priceAxis.min = minPrice - pricePadding;
            priceAxis.max = maxPrice + pricePadding;
            
            // 设置日期轴范围
            dateAxis.min = minDate;
            dateAxis.max = maxDate;
            
            // 设置成交量轴范围
            if (showVolume) {
                volumeAxis.max = maxVolume * 1.1;
            }
        }
    }
    
    // 设置数据模型
    function setModel(model) {
        marketDataModel = model;
        updateChart();
    }
    
    // 设置交易品种
    function setSymbol(newSymbol) {
        symbol = newSymbol;
        title = symbol + " 市场数据";
        updateChart();
    }
    
    // 缩放到特定时间范围
    function zoomToTimeRange(startDate, endDate) {
        dateAxis.min = startDate;
        dateAxis.max = endDate;
    }
    
    // 重置缩放
    function resetZoom() {
        updateChart();
    }
}