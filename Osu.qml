import QtQuick
import QtQuick.Layouts
import QtWebSockets
import QtQuick.Effects

Item {
    id: root
    width: 960
    height: 640

    // -- Configuration --
    property string socketUrl: "ws://192.168.0.4:24050/websocket/v2"
    //property string fontName: "Comfortaa"
    property string fontName: fontComfortaa.name

    // -- Colors (from CSS) --
    readonly property color cPrimary: "white"
    readonly property color cSecondary: "#ffffffd0"
    readonly property color cPp: "#537fd6"
    readonly property color cH100: "#9DF9AA"
    readonly property color cH50: "#F9EB9D"
    readonly property color cH0: "#F99D9D"
    readonly property color cSHD: "#E4E4E4"
    readonly property color cS: "#FFFB8B"
    readonly property color cA: "#9DF9AA"
    readonly property color cB: "#9DACF9"
    readonly property color cC: "#ED9DF9"
    readonly property color cD: "#F99D9D"

    // -- State Data --
    property string mapTitle: backend.osu.title
    property string mapArtist: backend.osu.artist
    property string diffName: backend.osu.difficulty

    property string ppCurrent: backend.osu.pp
    property int hit100: backend.osu.hits100
    property int hit50: backend.osu.hits50
    property int hit0: backend.osu.hitsMiss
    property string currentGrade: backend.osu

    // 0 = Menu, 2 = Playing, 7 = Results, etc.
    property int gameState: backend.osu.state
    property bool isGameplay: (gameState === 2 || gameState === 7
                               || gameState === 14)

    // -- UI Layer --

    // Background Image
    Image {
        id: bg
        anchors.fill: parent
        source: "http://192.168.0.4:24050/files/beatmap/background"
        fillMode: Image.PreserveAspectCrop
        scale: 1.1

        Connections {
            target: backend.osu
            onTrackChanged: {
                bg.source = "http://192.168.0.4:24050/files/beatmap/background?ts=" + Date.now()
            }
        }
    }

    MultiEffect {
        anchors.fill: parent
        source: bg
        blurEnabled: true
        blur: 0.6
        blurMax: 64
        brightness: -0.3
        scale: 1.2
    }

    // -- INFO SECTION (Center) --
    Item {
        id: infoContainer
        width: parent.width
        height: parent.height

        // JS Logic:
        // Gameplay: transform: translateY(-1.8rem) (Moves Up)
        // Menu: transform: translateY(-50%) (Centered)
        // We simulate this with 'y' positioning
        y: root.isGameplay ? -parent.height * 0.25 : 0

        Behavior on y {
            NumberAnimation {
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 10
            width: parent.width * 0.9

            Item {
                id: titleMarqueeContainer

                // Layout properties to fit within the ColumnLayout
                Layout.fillWidth: true
                Layout.preferredHeight: txtTitleArtist.height
                clip: true // Vital: Hides the text when it scrolls out of bounds

                Text {
                    id: txtTitleArtist
                    text: root.mapTitle + " <font color='#ffffffd0' size='5'>-</font> "
                          + root.mapArtist
                    font.family: root.fontName
                    font.pixelSize: 66
                    font.weight: Font.Light
                    color: root.cPrimary
                    style: Text.Outline
                    styleColor: "black"
                    textFormat: Text.RichText

                    // Don't wrap, we want it to overflow for calculation
                    wrapMode: Text.NoWrap

                    // -- Positioning Logic --
                    // If text fits: Center it manually using x calculation
                    // If text overflows: Animation takes control of 'x'
                    property bool overflows: paintedWidth > titleMarqueeContainer.width
                    x: overflows ? 0 : (titleMarqueeContainer.width - paintedWidth) / 2
                }

                // -- Scroll Animation --
                SequentialAnimation {
                    running: txtTitleArtist.overflows
                    loops: Animation.Infinite

                    // 1. Wait a moment at the start (text is at x=0)
                    PauseAnimation {
                        duration: 2000
                    }

                    // 2. Scroll to the left until the end is visible
                    NumberAnimation {
                        target: txtTitleArtist
                        property: "x"
                        from: 0
                        to: titleMarqueeContainer.width - txtTitleArtist.paintedWidth
                        // Calculate duration based on length so speed is consistent
                        duration: (txtTitleArtist.paintedWidth - titleMarqueeContainer.width) * 15
                        easing.type: Easing.Linear
                    }

                    // 3. Wait at the end
                    PauseAnimation {
                        duration: 2000
                    }

                    // 4. Scroll back to start (smooth return)
                    NumberAnimation {
                        target: txtTitleArtist
                        property: "x"
                        to: 0
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            // Map Info Row (Diff * RankIcon * Mapper BPM)
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Text {
                    text: root.diffName + "*"
                    color: root.cPrimary
                    font.family: root.fontName
                    font.pixelSize: 68
                    font.weight: Font.DemiBold
                    style: Text.Outline
                    styleColor: "black"
                }

                Image {
                    width: 68
                    height: 68
                    sourceSize.width: 68
                    sourceSize.height: 68
                    source: backend.osu.mapRank !== "" && backend.osu.rankColor
                            !== "" ? "image://fa/solid/" + backend.osu.mapRank
                                     + "?color=" + backend.osu.rankColor : ""
                }
                Text {
                    text: backend.osu.bpm + " BPM"
                    color: root.cPrimary
                    font.family: root.fontName
                    font.pixelSize: 68
                    font.weight: Font.DemiBold
                    style: Text.Outline
                    styleColor: "black"
                }
            }

            // PP Max (Visible only in Menu)
            Text {
                text: backend.osu.pp + " <font color='#537fd6' size='4'>pp</font>"
                font.family: root.fontName
                font.pixelSize: 70
                font.weight: Font.Bold
                color: root.cPrimary
                style: Text.Outline
                styleColor: "black"
                Layout.alignment: Qt.AlignHCenter
                visible: !root.isGameplay
                textFormat: Text.RichText
            }
        }
    }

    // -- GAMEPLAY CONTAINER (Bottom) --
    Item {
        id: gameplayContainer
        width: parent.width
        height: 400 // Approximate height for bottom bar

        // JS Logic:
        // Gameplay: bottom: 0 (Visible)
        // Menu: bottom: 50% (Hidden/Translated down)
        // Note: The CSS actually translates Y 100% to hide it.
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.isGameplay ? 0 : -height

        Behavior on anchors.bottomMargin {
            NumberAnimation {
                duration: 1000
                easing.type: Easing.InOutQuad
            }
        }

        RowLayout {
            anchors.centerIn: parent
            width: parent.width * 0.9
            spacing: 50

            // PP Current
            Column {
                Layout.preferredWidth: parent.width * 0.3
                spacing: 5

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: backend.osu.pp
                    font.family: root.fontName
                    font.pixelSize: 80
                    font.weight: Font.Bold
                    color: root.cPrimary
                    style: Text.Outline
                    styleColor: "black"
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "pp"
                    font.family: root.fontName
                    font.pixelSize: 64
                    font.weight: Font.Bold
                    color: root.cPp
                    style: Text.Outline
                    styleColor: "black"
                }
            }

            // Hits (100 / 50 / Miss)
            RowLayout {
                Layout.preferredWidth: parent.width * 0.4
                spacing: 30

                HitCounter {
                    label: root.hit100.toString()
                    color: root.cH100
                }
                HitCounter {
                    label: root.hit50.toString()
                    color: root.cH50
                }
                HitCounter {
                    label: root.hit0.toString()
                    color: root.cH0
                }
            }

            // Rank (SS/S/A...)
            Text {
                Layout.preferredWidth: parent.width * 0.3
                text: backend.osu.grade
                font.family: root.fontName
                font.pixelSize: 150
                font.weight: Font.Bold
                color: backend.osu.grade === "SS" || backend.osu.grade
                       === "S" ? cS : backend.osu.grade
                                 === "A" ? cA : backend.osu.grade
                                           === "B" ? cB : backend.osu.grade === "C" ? cC : cD
                style: Text.Outline
                styleColor: "black"
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    // -- Helper Component for Hits --
    component HitCounter: Column {
        property string label: "0"
        property color color: "white"

        spacing: 5
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: parent.label
            font.family: root.fontName
            font.pixelSize: 100
            font.weight: Font.Medium
            color: root.cPrimary
            style: Text.Outline
            styleColor: "black"
        }
        // The little colored underline bar
        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 40
            height: 10
            radius: 10
            color: parent.color
            border.width: 1 // slight shadow simulation
            border.color: "#00000040"
        }
    }
}
