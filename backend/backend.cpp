#include "backend.h"
#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

Backend::Backend(QObject *parent) :
    QObject(parent), m_spotify(parent)
{
    // Config load
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";

    if (!QFile::exists(configPath))
    {
        QString etcPath = "/etc/rpi_dashboard/config.ini";

        if (QFile::exists(etcPath))
        {
            configPath = etcPath;
        }
    }

    QSettings settings(configPath, QSettings::IniFormat);

    settings.beginGroup("PCServices");
    m_spotifyUrl = settings.value("spotify_ws_url").toString();
    m_spotifyAlbumUrl = settings.value("spotify_album_art_api").toString();
    m_prometheusUrl = settings.value("pc_prometheus_api").toString();
    m_pcFpsUrl = settings.value("pc_fps").toString();
    m_roomTempUrl = settings.value("room_temp").toString();
    m_tosuUrl = settings.value("tosu_ws").toString();
    settings.endGroup();

    settings.beginGroup("Weather");
    m_weatherUrl = settings.value("weather_ws_url").toString();
    settings.endGroup();

    m_spotify.setUrl(m_spotifyUrl, m_spotifyAlbumUrl);
    m_osu.setUrl(m_tosuUrl);

    //
    // --- WEATHER CLIENT ---
    //
    connect(&m_weather,
            &WeatherClient::weatherChanged,
            this,
            &Backend::weatherUpdated,
            Qt::UniqueConnection);

    m_weather.setServer(m_weatherUrl);
    m_weather.connectSocket();

    //
    // --- PROMETHEUS CLIENT ---
    //
    m_prom.setEndpoint(m_prometheusUrl);

    m_prom.addMetric("gpu_usage", "nvidia_smi_utilization_gpu_ratio");
    m_prom.addMetric("ram_usage",
                     "node_memory_MemAvailable_bytes");
    m_prom.addMetric("ram_total", "node_memory_MemTotal_bytes");
    m_prom.addMetric("gpu_temp", "nvidia_smi_temperature_gpu");
    m_prom.addMetric("boot_time", "node_boot_time_seconds");
    connect(&m_prom,
            &PrometheusClient::metricsChanged,
            this,
            &Backend::prometheusUpdated,
            Qt::UniqueConnection);

    m_prom_cpu_temp.setSingleRequest(true);
    m_prom_cpu_temp.setEndpoint(m_prometheusUrl);
    m_prom_cpu_temp.addMetric("cpu_temp", "node_hwmon_temp_celsius{sensor='temp13'}");

    m_prom_cpu_usage.setSingleRequest(true);
    m_prom_cpu_usage.setEndpoint(m_prometheusUrl);
    m_prom_cpu_usage.addMetric("cpu_usage", "100 - (avg(irate(node_cpu_seconds_total{mode='idle'}[1m])) * 100)");

    m_fps_updator.setServer(QUrl(m_pcFpsUrl));
    m_temp_updator.setServer(QUrl(m_roomTempUrl));
    m_temp_updator.start();

    connect(&m_promTimer, &QTimer::timeout, this, &Backend::pullPrometheus, Qt::UniqueConnection);
    connect(&m_fps_updator, &WSUpdator::valueChanged, this, &Backend::fpsUpdated, Qt::UniqueConnection);
    connect(&m_temp_updator, &WSUpdator::valueChanged, this, &Backend::roomTempUpdated, Qt::UniqueConnection);
    connect(&m_spotify, &Spotify::changed, this, &Backend::spotifyChanged, Qt::UniqueConnection);
    connect(&m_pingTimer, &QTimer::timeout, this, &Backend::checkPing, Qt::UniqueConnection);

    m_promTimer.setInterval(1000);
    m_pingTimer.setInterval(1000);
    m_pingTimer.start();

    pullPrometheus();

    startPcDependentServices();
}

//
// PROMETHEUS UPDATE
//
void
Backend::pullPrometheus()
{
    m_prom.update();
    m_prom_cpu_temp.update();
    m_prom_cpu_usage.update();
}

void
Backend::prometheusUpdated()
{
    auto data = m_prom.metrics();
    auto cpu_temp_data = m_prom_cpu_temp.metrics();
    auto cpu_usage_data = m_prom_cpu_usage.metrics();

    m_cpuUsage = cpu_usage_data["cpu_usage"].toDouble();
    m_gpuUsage = data["gpu_usage"].toDouble();
    m_ramUsage = data["ram_usage"].toDouble();
    m_cpuTemp = cpu_temp_data["cpu_temp"].toDouble();
    m_gpuTemp = data["gpu_temp"].toDouble();
    m_uptime = data["uptime"].toDouble();

    emit cpuUsageChanged();
    emit gpuUsageChanged();
    emit ramUsageChanged();
    emit cpuTempChanged();
    emit gpuTempChanged();
    emit uptimeChanged();
}

//
// WEATHER UPDATE
//
void
Backend::weatherUpdated()
{
    m_temperature = m_weather.temperature();
    m_description = m_weather.description();
    m_icon = m_weather.icon();
    m_uvIndex = m_weather.uvIndex();
    m_isDay = m_weather.day();

    emit weatherChanged();
}

void
Backend::fpsUpdated()
{
    m_fps = m_fps_updator.getValue().toDouble();
    emit fpsChanged();
}

void
Backend::roomTempUpdated()
{
    QString message = m_temp_updator.getValue();

    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "Failed to parse JSON:" << parseError.errorString();

        return;
    }

    if (!doc.isObject())
    {
        qWarning() << "JSON is not an object!";

        return;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("temperature")
        && obj["temperature"].isDouble())
    {
        m_room_temp = obj["temperature"].toDouble();
        emit roomTempChanged();
        //   qDebug() << "Parsed room temperature:" << m_room_temp;
    }
    else
    {
        qWarning() << "Temperature key missing or not a number!";
    }
}

void
Backend::checkPing()
{
    auto *pingProcess = new QProcess(this);

    connect(pingProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this, pingProcess](int exitCode, QProcess::ExitStatus)
            {
                pingProcess->deleteLater();

                if (exitCode == 0)
                {
                    m_pingFailures = 0;

                    if (!m_pcOnline)
                    {
                        m_pcOnline = true;
                        emit pcOnlineChanged();
                        startPcDependentServices();
                    }
                }
                else
                {
                    m_pingFailures++;

                    if (m_pingFailures >= 3
                        && m_pcOnline)
                    {
                        m_pcOnline = false;
                        emit pcOnlineChanged();
                        stopPcDependentServices();
                    }
                }
            }
            );

    pingProcess->start("ping",
                       QStringList() << "-c" << "1" << "-W" << "1" << "192.168.0.4");
}

void
Backend::startPcDependentServices()
{
    qDebug() << "Starting PC-dependent services";

    if (!m_promTimer.isActive())
    {
        m_promTimer.start();
    }

    // Pull immediately when back online
    pullPrometheus();

    if (!m_fps_updator.isRunning())
    {
        m_fps_updator.start();
    }

    m_osu.start();
}

void
Backend::stopPcDependentServices()
{
    qDebug() << "Stopping PC-dependent services";

    if (m_promTimer.isActive())
    {
        m_promTimer.stop();
    }

    if (m_fps_updator.isRunning())
    {
        m_fps_updator.stop();
    }

    m_osu.stop();
}
