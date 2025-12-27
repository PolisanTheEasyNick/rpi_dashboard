#include "weatherclient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

WeatherClient::WeatherClient(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[WeatherClient] Constructed";

    connect(&m_socket,
            &QWebSocket::connected,
            this,
            [this]()
            {
                qDebug() << "[WeatherClient] WebSocket connected";
            }
            );

    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WeatherClient::onMessageReceived, Qt::UniqueConnection);
    connect(&m_socket, &QWebSocket::disconnected, this, &WeatherClient::onClosed, Qt::UniqueConnection);

    connect(&m_socket,
            &QWebSocket::errorOccurred,
            this,
            [this](QAbstractSocket::SocketError e)
            {
                qWarning() << "[WeatherClient] WebSocket error:" << e << m_socket.errorString();
            }
            );
}

void
WeatherClient::setServer(const QString &url)
{
    m_server = url;
    qDebug() << "[WeatherClient] Server set to" << m_server;
}

void
WeatherClient::connectSocket()
{
    if (m_server.isEmpty())
    {
        qWarning() << "[WeatherClient] Server URL is empty, cannot connect";

        return;
    }

    qDebug() << "[WeatherClient] Connecting to" << m_server;
    m_socket.open(QUrl(m_server));
}

void
WeatherClient::onMessageReceived(const QString &msg)
{
    qDebug() << "[WeatherClient] Raw message received:" << msg;

    QJsonParseError parseError;
    auto jsonDoc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "[WeatherClient] JSON parse error:" << parseError.errorString();

        return;
    }

    if (!jsonDoc.isObject())
    {
        qWarning() << "[WeatherClient] JSON is not an object!";

        return;
    }

    auto json = jsonDoc.object();
    qDebug() << "[WeatherClient] Parsed JSON:" << json;

    if (json.contains("temperature_celsius")
        && json["temperature_celsius"].isDouble())
    {
        m_temperature = json["temperature_celsius"].toDouble();
    }

    if (json.contains("weather_text")
        && json["weather_text"].isString())
    {
        m_description = json["weather_text"].toString();
    }

    if (json.contains("weather_icon_id")
        && json["weather_icon_id"].isDouble())
    {
        qDebug() << "weather icon: " << json["weather_icon_id"];
        m_icon = QString::number(json["weather_icon_id"].toDouble());
    }

    if (json.contains("uv_index")
        && json["uv_index"].isDouble())
    {
        m_uvIndex = json["uv_index"].toInt();
    }

    if (json.contains("is_day"))
    {
        m_isDay = json["is_day"].toBool();
    }

    qDebug() << "[WeatherClient] Updated values:"
             << "Temp=" << m_temperature
             << "Desc=" << m_description
             << "Icon=" << m_icon
             << "UV=" << m_uvIndex
             << "IsDay=" << m_isDay;

    emit weatherChanged();
}

void
WeatherClient::onClosed()
{
    qWarning() << "[WeatherClient] WebSocket closed, reconnecting in 10s";
    QTimer::singleShot(10000, this, &WeatherClient::connectSocket);
}

double
WeatherClient::temperature() const { return m_temperature; }
QString
WeatherClient::description() const { return m_description; }
QString
WeatherClient::icon() const { return m_icon; }
int
WeatherClient::uvIndex() const { return m_uvIndex; }

bool
WeatherClient::day() const
{
    return m_isDay;
}
