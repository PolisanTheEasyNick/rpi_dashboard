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
            let isOsuRunning = backend.osu.started;
            let isPcOnline = backend.pcOnline;
            let isDay = backend.day;
            let isMusicPlaying = (backend.musicPlayer.isPlaying &&
                                 !backend.musicPlayer.gamemodeStarted &&
                                 backend.musicPlayer.musicPlayerStarted);

            if (isOsuRunning) {
                return osuView;
            }

            if (!isPcOnline) {
                if (isDay) {
                    return pcOffView; // Day + Offline
                } else {
                    return null;      // Night + Offline
                }
            }

            if (isMusicPlaying) {
                return musicPlayerView;
            }

            return dashboardView;
        }
    }

    Component {
        id: musicPlayerView
        MusicPlayer {}
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
