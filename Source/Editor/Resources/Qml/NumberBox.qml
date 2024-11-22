import QtQuick;
import QtQuick.Controls;

Item{
    id: control
    implicitHeight: 25
    implicitWidth: 150
    property bool isLimited: false
    property bool isHovered: false
    property var min:0
    property var max:100
    property var number: 0
    property real step : 1
    property int precision: 3
    signal valueChanged(number:var)
    function setNumber(value:var){
        if(value !== number && !isNaN(value)){
            number = value
            if(isLimited){
                if(number>max){
                    number = max
                }
                if(number<min){
                    number = min
                }
            }
            valueChanged(number)
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        border.color: "transparent"
        border.width: 1
        clip: true

        Rectangle{
            visible: isHovered && isLimited
            anchors.fill: parent
            anchors.rightMargin:  parent.width * (max - number) /(max - min)
            color: "red"
        }
        TextInput{
            id: input
            enabled: false
            anchors.fill: parent
            anchors.leftMargin: 5
            anchors.rightMargin: 5
            text: helper.numberToString(number,precision)
            verticalAlignment: Text.AlignVCenter
            validator: DoubleValidator{}
            onEditingFinished:{
                enabled = false
                dragArea.visible = true
                dragArea.cursorShape = Qt.SplitHCursor
                var newVar = parseFloat(input.text)
                if(isNaN(newVar)){
                    text = helper.numberToString(number,precision)
                }
                else{
                    control.setNumber(parseFloat(input.text))
                }
            }
        }

        MouseArea{
            id: dragArea
            property real lastPressX : -1
            property real lastPressY : -1
            anchors.fill: parent
            hoverEnabled : true
            cursorShape: Qt.SplitHCursor
            onDoubleClicked: {
                cursorShape =  Qt.ArrowCursor
                input.enabled = true
                dragArea.visible = false
                input.forceActiveFocus()
                input.selectAll()
            }
            onClicked: {
                input.forceActiveFocus()
            }
            onEntered:{
                isHovered = true
                exitAnimation.stop()
                enterAnimation.start()
            }
            onExited:{
                isHovered = false
                enterAnimation.stop()
                exitAnimation.start()
            }
            onPressed:
                (mouse)=>{
                    if(mouse.button === Qt.LeftButton){
                        lastPressX = mouse.x
                        lastPressY = mouse.y
                        cursorShape = Qt.BlankCursor
                        var global = dragArea.mapToGlobal(lastPressX,lastPressY)
                        //console.log("press last:",lastPressX,lastPressY,"  global:",global)
                    }
            }
            onReleased:
                (mouse)=>{
                    lastPressX = -1
                    lastPressY = -1
                    cursorShape = Qt.SplitHCursor
                    console.log("Release")
                }
            onPositionChanged:
                (mouse)=>{
                    if(!input.enabled && mouse.buttons&Qt.LeftButton){
                        var offset = mouse.x - lastPressX
                        setNumber(number + offset * step)
                        var global = dragArea.mapToGlobal(lastPressX,lastPressY)
                        var local = dragArea.mapFromGlobal(global.x,global.y)
                        console.log("last:",lastPressX,lastPressY,"  global:",global," local:",local)
                        helper.setCursorPos(global.x,global.y)
                    }
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
