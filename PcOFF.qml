import QtQuick
import QtQuick.Layouts
import "components/Utils.js" as ColorUtils

Item {
    id: root
    anchors.fill: parent

    property date currentDate: new Date()

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: root.currentDate = new Date()
    }

    // --- TOP SECTION ---
    Item {
        id: topSection
        height: parent.height * 0.4
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 40

        // 1) Date & Time (Left)
        Column {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            spacing: 5

            // Anchoring children to horizontalCenter of the Column aligns them centrally
            Text {
                text: Qt.formatDateTime(root.currentDate, "MMMM d, yyyy")
                font.pixelSize: 60
                font.family: fontBinary.name
                color: Style.textWhite
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: Qt.formatDateTime(root.currentDate, "hh:mm:ss")
                font.pixelSize: 55
                font.family: fontBinary.name
                color: Style.textWhite
                opacity: 0.8
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        // 2) Room Temperature (Right)
        Row {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 5

            Text {
                text: backend.room_temp.toFixed(2)
                font.pixelSize: 75
                font.family: fontBinary.name

                color: ColorUtils.roomTempColor(backend.room_temp)

                Behavior on color {
                    ColorAnimation {
                        duration: 300
                    }
                }
            }

            Text {
                text: "°C"
                font.pixelSize: 55
                font.family: fontBinary.name
                color: Style.textWhite
                anchors.baseline: parent.bottom // Align "C" to baseline of the number
            }
        }
    }

    // --- BOTTOM SECTION (Weather) ---
    Item {
        id: weatherSection
        height: parent.height * 0.5
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 20
        anchors.bottomMargin: 80

        // MAIN VERTICAL CONTAINER (Top row + Description below)
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 10

            // --- TOP ROW: Icon | Temp | UV ---
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                //spacing: 30
                //Layout.leftMargin: -50

                // 1) Icon (Most Left)
                AnimatedImage {
                    source: "qrc:/weather/weather/"
                            + (backend.icon !== "" ? backend.icon : "1") + ".gif"

                    Layout.preferredWidth: 350
                    Layout.preferredHeight: 350
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    Layout.leftMargin: -75
                }

                // 2) Temperature (Next to Icon)
                Row {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 0
                    Layout.leftMargin: -50

                    Text {
                        text: backend.temperature
                        font.pixelSize: 120
                        font.family: fontBinary.name
                        color: ColorUtils.outsideTempColor(backend.temperature)

                        Behavior on color {
                            ColorAnimation {
                                duration: 400
                            }
                        }
                    }
                    Text {
                        text: "°C"
                        font.pixelSize: 100
                        font.family: fontBinary.name
                        color: Style.textWhite
                        anchors.baseline: parent.bottom
                        anchors.bottomMargin: 20
                    }
                }

                // 3) UV (Right after Temp, Image next to Number)
                Row {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 10 // Space between UV icon and number

                    Image {
                        source: "qrc:/weather/weather/uv-index.svg"
                        sourceSize: Qt.size(img.sourceSize.width * 3,
                                            img.sourceSize.height * 3)
                        Image {
                            id: img
                            source: parent.source
                            width: 0
                            height: 0
                        }
                    }

                    Text {
                        text: backend.uv ? backend.uv : "0"
                        font.pixelSize: 100
                        font.family: fontBinary.name
                        color: Style.textWhite
                        anchors.verticalCenter: parent.verticalCenter // Align text to icon center
                    }
                }
            }

            // --- BOTTOM ITEM: Description ---
            // 4) Description (Under all that, new line)
            Text {
                Layout.alignment: Qt.AlignHCenter // Center the description
                text: backend.description
                font.pixelSize: 100
                font.family: fontBinary.name
                color: Style.textWhite
                font.capitalization: Font.Capitalize
                Layout.topMargin: -50
            }
        }
    }
}
