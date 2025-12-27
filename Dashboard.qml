import QtQuick
import QtQuick.Layouts
import rpi_dashboard

GridLayout {
    columns: 3
    rowSpacing: 5
    columnSpacing: 5
    //anchors.fill: parent
    anchors.margins: 5

    Gauge {
        textLabel: "CPU Usage"
        value: backend.cpuUsage
    }
    Gauge {
        textLabel: "GPU Usage"
        value: backend.gpuUsage
    }
    Gauge {
        textLabel: "RAM Usage"
        value: backend.ramUsage
    }

    Gauge {
        textLabel: "CPU Temp"
        value: backend.cpuTemp
        suffix: "°C"
    }
    Gauge {
        textLabel: "GPU Temp"
        value: backend.gpuTemp
        suffix: "°C"
    }

    GridLayout {
        columns: 1
        rowSpacing: 5
        columnSpacing: 5

        Uptime {
            Layout.fillWidth: true
            time: backend.uptime
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1
            spacing: 5

            InfoTile {
                label: "FPS"
                value: backend.fps
                Layout.fillWidth: true
                // Layout.fillHeight: true
                Layout.preferredWidth: 2
            }
            InfoTile {
                label: "UV"
                value: backend.uvIndex
                warningThreshold: 3
                badThreshold: 5
                thresholdReversed: false
                Layout.fillWidth: true
                //  Layout.fillHeight: true
                Layout.preferredWidth: 2
            }
            InfoTile {
                label: "Room"
                value: backend.room_temp.toFixed(2)
                warningThreshold: 3
                badThreshold: 5
                thresholdReversed: false
                useTemperatureGradient: true
                Layout.fillWidth: true
                //   Layout.fillHeight: true
                Layout.preferredWidth: 4
            }
        }
    }

    WeatherTile {
        width: Style.gaugeWidth
    }
}
