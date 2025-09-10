import QtQuick 2.15
import QtQuick.Window 2.15
import QtCharts 2.15


Window {
    width: 800
    height: 600
    title: "kquant"
    property  string  dateFormat: "yyyy-MM-dd hh:mm:ss"
    visible:true
    function getTime(){
        // 21 May 2001 14:13:09
        var dateTime = new Date(2014, 5, 21, 14, 13, 09)
        return dateTime
    }
    ChartView {
        title: "Candlestick Series"
        anchors.centerIn: parent
        width: parent.width
        height: parent.height * 0.8
        visible: true


        CandlestickSeries {
            name: "Acme Ltd."

            DateTimeAxis {
                id: xAxis
                format:dateFormat

                min: Qt.formatDateTime(getTime(),dateFormat)
                // tickCount: 10
                max: Qt.formatDateTime(new Date(),dateFormat)
            }
            ValueAxis {
                id: yAxis
                min: 100
                tickCount: 10
                max: 1000
            }

            axisX:  xAxis
            axisY : yAxis


            increasingColor: "green"
            decreasingColor: "red"

            CandlestickSet { timestamp: 1435708800000; open: 690; high: 694; low: 599; close: 660 }
            CandlestickSet { timestamp: 1435795200000; open: 669; high: 669; low: 669; close: 669 }
            CandlestickSet { timestamp: 1436140800000; open: 485; high: 623; low: 485; close: 600 }
            CandlestickSet { timestamp: 1436227200000; open: 589; high: 615; low: 377; close: 569 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313670000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313800000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436314600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1446313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1456313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1466313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1476313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1486313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1496313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1536413600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1636313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1736313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1836313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1936513600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436613600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436317600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436313600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436318600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436310600000; open: 464; high: 464; low: 254; close: 254 }
            // CandlestickSet { timestamp: 1436710600000; open: 464; high: 464; low: 254; close: 254 }
        }
    }

}
