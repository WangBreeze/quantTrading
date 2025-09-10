import QtQuick 2.15
import QtQuick.Controls 2.15

Label {
    id: root
    text: "Label"
    font.family: "楷体"
    font.pixelSize: 14
    color: "black"
    
    property alias textColor: root.color
    property alias fontSize: root.font.pixelSize
}