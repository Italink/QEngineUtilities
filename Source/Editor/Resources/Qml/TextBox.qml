import QtQuick;
import QtQuick.Controls;

Item{
    id: control
    property var value
    implicitHeight: 25
    signal asValueChanged(text:var)
    function setValue(newText:var){
        if(newText !== value){
            value = newText
            asValueChanged(value)
        }
    }
    Rectangle {
        anchors.fill: parent
        border.color: "transparent"
        border.width: 1
        clip: true
        MouseArea{
            id: dragArea
            hoverEnabled: true
            anchors.fill: parent
            onEntered:{
                exitAnimation.stop()
                enterAnimation.start()
            }
            onExited:{
                enterAnimation.stop()
                exitAnimation.start()
            }
        }
        TextInput{
            id: lineEditor
            enabled: true
            clip: true
            anchors.fill: parent
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            text: control.value
            wrapMode: TextInput.WordWrap
            verticalAlignment: Text.AlignVCenter
            onEditingFinished:{
                setValue(lineEditor.text)
            }
        }
        ColorAnimation on border.color{
            id: enterAnimation
            to: "red"
            duration: 100
            running: false
        }
        ColorAnimation on border.color{
            id: exitAnimation
            to: "transparent"
            duration: 100
            running: false
        }
    }
}

