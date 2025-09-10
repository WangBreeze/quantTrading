import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: root
    width: 100
    height: 40
    text: "Button"
    font.family: "楷体"
    font.pixelSize: 14
    
    property alias backgroundColor: background.color
    property alias textColor: contentItem.color
    
    background: Rectangle {
        id: background
        radius: 5
        color: "white"
    }
    
    contentItem: Text {
        id: contentItem
        text: root.text
        font: root.font
        color: "black"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
