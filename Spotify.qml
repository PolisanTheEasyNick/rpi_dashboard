import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5
import QtQuick.Effects 6.5

Item {
    id: root
    width: 960
    height: 640

    property url coverSource: backend.spotify.artURL
    property string songTitle: backend.spotify.title
    property string songArtist: backend.spotify.artist
    property string songAlbum: backend.spotify.album

    property bool imageReady: bgImage.status === Image.Ready

    Image {
        id: bgImage
        anchors.fill: parent
        source: coverSource
        fillMode: Image.PreserveAspectCrop
        smooth: true
        scale: 2
        transformOrigin: Item.Center
    }

    MultiEffect {
        anchors.fill: parent
        source: bgImage
        blurEnabled: true
        blur: 1.0
        blurMax: 64
        brightness: -0.2
        scale: 2

        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: 50000
            loops: Animation.Infinite
            running: true
        }
    }

    RowLayout {
        anchors.centerIn: parent
        spacing: imageReady ? 15 : 0

        Image {
            visible: imageReady
            source: coverSource
            Layout.preferredWidth: 500
            Layout.preferredHeight: 500
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        Column {
            Layout.preferredWidth: imageReady ? 380 : root.width * 0.8
            spacing: 10
            // center all texts vertically inside Column
            //anchors.verticalCenter: parent.verticalCenter
            Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

            Text {
                font.family: fontLexend.name
                visible: songTitle != ""
                width: parent.width
                horizontalAlignment: imageReady ? Text.AlignLeft : Text.AlignHCenter
                text: songTitle
                font.pixelSize: 55
                color: "white"
                wrapMode: Text.WordWrap
                maximumLineCount: 3
                elide: Text.ElideRight
                style: Text.Outline
                styleColor: "black"
            }

            Item {
                visible: songArtist != ""
                width: parent.width
                height: 20
            }

            Text {
                font.family: fontLexend.name
                visible: songArtist != ""
                width: parent.width
                horizontalAlignment: imageReady ? Text.AlignLeft : Text.AlignHCenter
                text: songArtist
                font.pixelSize: 40
                color: "white"
                elide: Text.ElideRight
                maximumLineCount: 1
                style: Text.Outline
                styleColor: "black"
            }

            Item {
                visible: songAlbum != ""
                width: parent.width
                height: 70
            }

            Text {
                font.family: fontLexend.name
                visible: songAlbum != ""
                width: parent.width
                horizontalAlignment: imageReady ? Text.AlignLeft : Text.AlignHCenter
                text: songAlbum
                font.pixelSize: 35
                color: "#c1c1c1"
                elide: Text.ElideRight
                maximumLineCount: 1
                style: Text.Outline
                styleColor: "black"
            }
        }
    }
}
