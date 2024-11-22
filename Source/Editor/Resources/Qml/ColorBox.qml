import QtQuick;
import QtQuick.Controls;

Item{
    id: control
    property color value
    implicitHeight: 25
    signal asValueChanged(text:var)
    signal asClicked()

    function setValue(newValue:var){
        if(newValue !== value){
            value = newValue
            asValueChanged(value)
        }
    }
    Button{
        anchors.margins:  2
        anchors.fill: parent
        background: Rectangle {
            color: value
        }
        onClicked: {
            asClicked()
        }
    }
}

