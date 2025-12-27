#ifndef BACKEND_H
#define BACKEND_H

#include "backend/osu.h"
#include "backend/spotify.h"
#include "backend/wsupdator.h"
#include "prometheusclient.h"
#include "weatherclient.h"
#include <QObject>
#include <QTimer>

class Backend :
    public QObject
{
    Q_OBJECT

    // System metrics (Prometheus)
    Q_PROPERTY(double cpuUsage   READ cpuUsage   NOTIFY cpuUsageChanged)
    Q_PROPERTY(double gpuUsage   READ gpuUsage   NOTIFY gpuUsageChanged)
    Q_PROPERTY(double ramUsage   READ ramUsage   NOTIFY ramUsageChanged)
    Q_PROPERTY(double cpuTemp    READ cpuTemp    NOTIFY cpuTempChanged)
    Q_PROPERTY(double gpuTemp    READ gpuTemp    NOTIFY gpuTempChanged)
    Q_PROPERTY(unsigned int uptime READ uptimeGet NOTIFY uptimeChanged)

    // Weather data (WeatherClient)
    Q_PROPERTY(double temperature READ temperature NOTIFY weatherChanged)
    Q_PROPERTY(QString description READ description NOTIFY weatherChanged)
    Q_PROPERTY(QString icon        READ icon        NOTIFY weatherChanged)
    Q_PROPERTY(int uvIndex        READ uvIndex      NOTIFY weatherChanged)
    Q_PROPERTY(bool day READ isDay NOTIFY weatherChanged)

    // FPS
    Q_PROPERTY(quint8 fps        READ FPS      NOTIFY fpsChanged)

    // Room temp
    Q_PROPERTY(double room_temp        READ roomTemp      NOTIFY roomTempChanged)

    // PC Status (Ping)
    Q_PROPERTY(bool pcOnline      READ pcOnline     NOTIFY pcOnlineChanged)

    // Spotify
    Q_PROPERTY(Spotify* spotify READ spotify CONSTANT)

    // Osu
    Q_PROPERTY(Osu* osu READ osu CONSTANT)

public:
    explicit Backend(QObject *parent = nullptr);
    Spotify *spotify() { return &m_spotify; }
    Osu *osu() { return &m_osu; }

    // Getters
    double
    cpuUsage() const { return m_cpuUsage; }
    double
    gpuUsage() const { return m_gpuUsage; }
    double
    ramUsage() const { return m_ramUsage; }
    double
    cpuTemp()  const { return m_cpuTemp; }
    double
    gpuTemp()  const { return m_gpuTemp; }
    double
    uptimeGet() const { return m_uptime; }

    double
    temperature() const { return m_temperature; }
    QString
    description() const { return m_description; }
    QString
    icon() const { return m_icon; }
    int
    uvIndex() const { return m_uvIndex; }
    quint8
    FPS() const { return m_fps; }
    double
    roomTemp() const { return m_room_temp; }
    bool
    pcOnline() const { return m_pcOnline; }
    bool
    isDay() const {return m_isDay;}

signals:
    void cpuUsageChanged();
    void gpuUsageChanged();
    void ramUsageChanged();
    void cpuTempChanged();
    void gpuTempChanged();
    void uptimeChanged();
    void weatherChanged();
    void fpsChanged();
    void rpiChanged();
    void roomTempChanged();
    void spotifyChanged();
    void pcOnlineChanged();

private slots:
    void pullPrometheus();          // Timer to refresh Prometheus data
    void prometheusUpdated();       // Called when PrometheusClient updates
    void weatherUpdated();          // Called when WeatherClient updates
    void fpsUpdated();
    void roomTempUpdated();
    void checkPing();

    // void onSpotifyChanged();

private:
    PrometheusClient m_prom, m_prom_cpu_usage, m_prom_cpu_temp;
    WSUpdator m_fps_updator, m_temp_updator;
    WeatherClient m_weather;

    QTimer m_promTimer;
    QTimer m_pingTimer;

    // Prometheus values
    double m_cpuUsage = 0;
    double m_gpuUsage = 0;
    double m_ramUsage = 0;
    double m_cpuTemp = 0;
    double m_gpuTemp = 0;
    unsigned int m_uptime = 0;
    double m_room_temp = 0;
    quint8 m_fps = 0;

    // Weather values
    double m_temperature = 0;
    QString m_description;
    QString m_icon;
    int m_uvIndex = 0;
    bool m_isDay = 0;

    bool m_pcOnline = true;
    int m_pingFailures = 0;

    // Spotify
    Spotify m_spotify;

    Osu m_osu;

    void startPcDependentServices();
    void stopPcDependentServices();

    // config
    QString m_spotifyUrl;
    QString m_spotifyAlbumUrl;
    QString m_prometheusUrl;
    QString m_pcFpsUrl;
    QString m_roomTempUrl;
    QString m_tosuUrl;
    QString m_weatherUrl;
};

#endif // BACKEND_H
