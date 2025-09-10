import QtQuick 2.15
import QtQuick.Controls 2.15

ScrollView {
    id: root
    width: 200
    height: 100
    
    property alias text: textArea.text
    property alias textColor: textArea.color
    property alias fontSize: textArea.font.pixelSize
    property alias backgroundColor: background.color
    
    background: Rectangle {
        id: background
        color: "white"
    }
    
    TextArea {
        id: textArea
        wrapMode: Text.Wrap
        maximumLength: 20000
        font.family: "楷体"
        font.pixelSize: 14
        color: "black"
    }
}