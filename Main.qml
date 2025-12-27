import QtQuick
import QtQuick.Controls

Window {
    id: main
    width: 960
    height: 640
    visible: true
    title: qsTr("Raspi Monitor")
    color: Style.background

    visibility: Window.FullScreen
    flags: Qt.FramelessWindowHint
    property bool pcOnline: true

    FontLoader {
        id: fontBinary
        source: "qrc:/assets/fonts/BinaryNeue.ttf"
    }
    FontLoader {
        id: fontFunnel
        source: "qrc:/assets/fonts/FunnelDisplay.ttf"
    }
    FontLoader {
        id: fontLexend
        source: "qrc:/assets/fonts/Lexend.ttf"
    }

    FontLoader {
        id: fontComfortaa
        source: "qrc:/assets/fonts/Comfortaa.woff"
    }

    Loader {
        id: mainLoader
        anchors.left: main.left
        active: true

        sourceComponent: {
            if (backend.osu.started) {
                return osuView
            }

            // 1. Priority: Spotify (Overrides everything if playing)
            if (backend.spotify.isPlaying && !backend.spotify.gamemodeStarted
                    && backend.spotify.spotifyStarted) {
                return spotifyView
            }

            // 2. Priority: PC is Offline
            if (backend.pcOnline === false) {
                if (backend.day === true) {
                    // Day + Offline -> Show Info Screen
                    return pcOffView
                } else {
                    // Night + Offline -> Show NOTHING (Unload loader)
                    // This leaves just the Window background color
                    return null
                }
            }

            // 3. Default: Dashboard (PC is Online)
            return dashboardView
        }
    }
    Component {
        id: spotifyView
        Spotify {}
    }

    Component {
        id: dashboardView
        Dashboard {}
    }

    Component {
        id: pcOffView
        PcOFF {}
    }

    Component {
        id: osuView
        Osu {}
    }
}
