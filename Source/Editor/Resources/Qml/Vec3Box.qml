import QtQuick;
import QtQuick.Controls;
import QtQuick.Layouts;

Item{
    id: control
    property vector3d value
    implicitHeight: 25
    signal asValueChanged(value:var)
    function setValue(newValue:var){
        if(value !== newValue){
            value = newValue
            asValueChanged(value)
        }
    }
    RowLayout{
        anchors.fill: parent
        NumberBox{
            id: xBox
            width: parent.width/4
            Layout.alignment: Qt.AlignLeft
            onNumberChanged: {
                value.x = number
                asValueChanged(value)
            }
        }
        NumberBox{
            id: yBox
            width: parent.width/4
            Layout.alignment: Qt.AlignLeft
            onNumberChanged: {
                value.y = number
                asValueChanged(value)
            }
        }
        NumberBox{
            id: zBox
            width: parent.width/4
            Layout.alignment: Qt.AlignLeft
            onNumberChanged: {
                value.z = number
                asValueChanged(value)
            }
        }
        Item {
               Layout.fillWidth: true
        }
    }
}
