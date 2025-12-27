import QtQuick
import QtQuick.Shapes

Item {
    id: gauge

    property real value: 0
    property real maximum: 100
    property real minimum: 0

    property real startAngle: 160
    property real sweepAngle: 220

    property string textLabel: "Label"
    property string suffix: "%"
    property bool showValue: true
    property color barColor: Style.good

    property real warningThreshold: 80
    property real badThreshold: 90

    width: Style.blockWidth
    height: Style.blockHeight + 10

    readonly property real progress: (Math.max(
                                          minimum, Math.min(
                                              maximum,
                                              value)) - minimum) / (maximum - minimum)

    readonly property color gaugeColor: {
        if (value < warningThreshold)
            return Style.good
        else if (value >= warningThreshold && value < badThreshold)
            return Style.warning
        else
            return Style.bad
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: Style.borderColor
        border.width: 2
    }

    Text {
        text: gauge.textLabel
        color: "white"
        font.pixelSize: 40
        font.family: fontLexend.name
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        z: 2
    }

    Shape {
        id: shape
        anchors.fill: parent

        layer.enabled: true
        layer.samples: 4

        ShapePath {
            strokeColor: Style.dialColor
            strokeWidth: 28
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: gauge.width / 2
                centerY: gauge.height * 0.75
                radiusX: (gauge.width / 2) - 20
                radiusY: (gauge.width / 2) - 20
                startAngle: gauge.startAngle
                sweepAngle: gauge.sweepAngle
            }
        }

        ShapePath {
            strokeColor: gaugeColor
            strokeWidth: 32
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap

            PathAngleArc {
                centerX: gauge.width / 2
                centerY: gauge.height * 0.75
                radiusX: (gauge.width / 2) - 20
                radiusY: (gauge.width / 2) - 20
                startAngle: gauge.startAngle
                sweepAngle: gauge.sweepAngle * gauge.progress
            }
        }
    }

    Text {
        visible: gauge.showValue
        text: Math.round(gauge.value) + gauge.suffix
        color: gaugeColor
        font.pixelSize: 75
        font.family: fontBinary.name
        anchors.centerIn: parent
        anchors.verticalCenterOffset: 80

        Behavior on color {
            ColorAnimation {
                duration: 200
            }
        }
    }

    Behavior on value {
        NumberAnimation {
            duration: 1000
            easing.type: Easing.OutCubic
        }
    }
}
