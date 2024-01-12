import QtQuick;
import QtQuick.Controls;

Item{
    id: control
    property var value
    property var model
    implicitHeight: 25
    signal asValueChanged(value:var)
    function setValue(newValue:var){
        if(newValue !== value){
            value = newValue
            asValueChanged(value)
        }
    }
    Rectangle {
        anchors.fill: parent
        border.color: "transparent"
        border.width: 1
        clip: true
        ComboBox {
            anchors.margins: 2
            anchors.fill: parent
            model: control.model
            onCurrentTextChanged: {
                setValue(currentText)
            }
         }
    }
}

