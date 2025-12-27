import QtQuick
import "Utils.js" as ColorUtils

Item {
    id: weather
    width: Style.gaugeWidth
    height: Style.gaugeHeight / 2

    property string temperature: backend.temperature
    property string description: backend.description
    property url iconSource: "qrc:/weather/weather/"
                             + (backend.icon !== "" ? backend.icon : "1") + ".gif"

    Row {
        anchors.fill: parent
        anchors.margins: 5
        anchors.leftMargin: -40

        spacing: 10
        // vertically center all children
        anchors.verticalCenter: parent.verticalCenter

        // --- Weather Icon ---
        AnimatedImage {
            source: iconSource
            width: 200
            height: 200
            fillMode: Image.PreserveAspectFit
            smooth: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 50
        }

        // --- Temperature Column ---
        Column {
            spacing: 25
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 50

            Row {
                spacing: 0
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.horizontalCenterOffset: -20
                Text {
                    text: weather.temperature
                    font.pixelSize: 75
                    font.family: fontBinary.name
                    color: ColorUtils.outsideTempColor(weather.temperature)

                    Behavior on color {
                        ColorAnimation {
                            duration: 400
                        }
                    }
                }
                Text {
                    text: "°C"
                    font.pixelSize: 60
                    font.family: fontBinary.name
                    color: Style.textWhite
                    anchors.bottom: parent.bottom
                }
            }
        }

        // --- Description ---
        Text {
            text: weather.description
            font.pixelSize: 80
            font.family: fontBinary.name
            color: Style.textWhite
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: 50
        }
    }
}
