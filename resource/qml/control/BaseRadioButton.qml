import QtQuick 2.15
import QtQuick.Controls 2.15

RadioButton {
    id: root
    text: "Option"
    font.family: "楷体"
    font.pixelSize: 14
    
    property alias textColor: contentItem.color
    property alias fontSize: contentItem.font.pixelSize
    
    indicator: Rectangle {
        implicitWidth: 16
        implicitHeight: 16
        radius: 8
        border.color: root.checked ? "blue" : "gray"
        border.width: 1
        
        Rectangle {
            anchors.fill: parent
            anchors.margins: 4
            radius: 4
            color: "blue"
            visible: root.checked
        }
    }
    
    contentItem: Text {
        id: contentItem
        text: root.text
        font: root.font
        color: "black"
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.indicator.width + root.spacing
    }
}