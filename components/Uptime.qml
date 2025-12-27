import QtQuick

Rectangle {
    id: uptime

    property string label: "UPTIME"
    property string time: "00:00:00"

    width: Style.blockWidth
    height: Style.blockHeight / 2
    color: "transparent"
    border.color: Style.borderColor
    border.width: 1

    function formatTime(seconds) {
        var h = Math.floor(seconds / 3600)
        var m = Math.floor((seconds % 3600) / 60)
        var s = Math.floor(seconds % 60)

        return ("0" + h).slice(-2) + ":" + ("0" + m).slice(
                    -2) + ":" + ("0" + s).slice(-2)
    }

    Column {
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        spacing: 5

        Text {
            text: uptime.label
            color: Style.textWhite
            font.pixelSize: 40
            font.family: fontLexend.name
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: formatTime(uptime.time)
            color: Style.textWhite
            font.pixelSize: 70
            font.family: fontBinary.name
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
