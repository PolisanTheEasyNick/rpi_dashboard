import QtQuick
import "Utils.js" as ColorUtils

Rectangle {
    id: info
    width: Style.blockWidth
    height: Style.blockHeight / 2
    color: "transparent"
    border.color: Style.borderColor
    border.width: 1

    property string label: "FPS"
    property real value: 10

    property bool useTemperatureGradient: false

    property real warningThreshold: 50
    property real badThreshold: 30
    property bool thresholdReversed: true

    readonly property color fontColor: {
        if (useTemperatureGradient) {
            if (value < 18) {
                return Style.cold
            } else if (value <= 21) {
                var t = (value - 18) / (21 - 18)
                return ColorUtils.lerpColor(Style.cold, Style.good, t)
            } else if (value <= 27) {
                var t2 = (value - 21) / (27 - 21)
                return ColorUtils.lerpColor(Style.good, Style.warm, t2)
            } else {
                return Style.hot
            }
        } else {
            if (thresholdReversed) {
                if (value >= warningThreshold || value === 0)
                    return Style.good
                if (value >= badThreshold)
                    return Style.warning
                return Style.bad
            } else {
                if (value < warningThreshold)
                    return Style.good
                if (value < badThreshold)
                    return Style.warning
                return Style.bad
            }
        }
    }

    Text {
        id: labelText
        text: label
        color: Style.textWhite
        font.pixelSize: 30
        font.family: fontLexend.name

        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
    }

    Text {
        text: value
        color: fontColor
        font.pixelSize: 50
        font.family: fontBinary.name

        anchors {
            horizontalCenter: parent.horizontalCenter
            top: parent.top
            topMargin: 50
        }

        verticalAlignment: Text.AlignVCenter

        Behavior on color {
            ColorAnimation {
                duration: 300
            }
        }
    }
}
