#include "prometheusclient.h"

#include "prometheusclient.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>

PrometheusClient::PrometheusClient(QObject *parent, bool is_single_request) :
    QObject(parent)
{
    m_is_single_request = is_single_request;
    // m_reply = nullptr;
}

QString
PrometheusClient::endpoint() const { return m_endpoint; }

void
PrometheusClient::setEndpoint(const QString &url)
{
    if (m_endpoint != url)
    {
        m_endpoint = url;
        emit endpointChanged();
    }
}

void
PrometheusClient::setSingleRequest(bool isSingle)
{
    m_is_single_request = isSingle;
}

QVariantMap
PrometheusClient::metrics() const
{
    return m_values;
}

void
PrometheusClient::addMetric(const QString &key, const QString &query)
{
    m_queries[key] = query;
}

void
PrometheusClient::update()
{
    if (m_endpoint.isEmpty()
        || m_queries.isEmpty())
    {
        return;
    }

    if (!m_is_single_request)
    {
        QStringList metricNames;

        for (const auto &v : m_queries.values())
        {
            metricNames << v.toString();
        }

        QString regex = metricNames.join("|");
        // qDebug() << "[Prometheus] Regex: " << regex;
        QString finalQuery = QString("{__name__=~\"%1\"}").arg(regex);
        // QUrl url(m_endpoint + "?query=" + QUrl::toPercentEncoding(finalQuery) + "&time=" +
        //          QString::number(QDateTime().currentSecsSinceEpoch()));
        QUrl url(m_endpoint + "?query=" + QUrl::toPercentEncoding(finalQuery));

        // qDebug() << "[Prometheus] Requesting:" << url;
        QNetworkRequest req(url);
        req.setTransferTimeout(1000);
        auto *reply = m_mgr.get(req);
        connect(reply,
                &QNetworkReply::finished,
                this,
                [this, reply]
                {
                    handleResponse(reply);
                    reply->deleteLater();
                }
                );
    }
    else
    {
        if (m_queries.values().size() > 0)
        {
            // QUrl url(m_endpoint + "?query=" + QUrl::toPercentEncoding(m_queries.values()[0].toString()) + "&time=" +
            //          QString::number(QDateTime().currentSecsSinceEpoch()));
            QUrl url(m_endpoint + "?query=" + QUrl::toPercentEncoding(m_queries.values()[0].toString()));
            // qDebug() << "[Prometheus] Requesting:" << url;
            auto *reply = m_mgr.get(QNetworkRequest(url));
            connect(reply,
                    &QNetworkReply::finished,
                    this,
                    [this, reply]
                    {
                        handleResponse(reply);
                        reply->deleteLater();
                    }
                    );
        }
    }
}

void
PrometheusClient::handleResponse(QNetworkReply *reply)
{
    if (!reply)
    {
        return;
    }

    if (reply->error() != QNetworkReply::NoError)
    {
        return;
    }

    QByteArray data = reply->readAll();
    // qDebug() << "[Prometheus] Raw response:" << data;

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (!doc.isObject())
    {
        qWarning() << "[Prometheus] Invalid JSON received";

        return;
    }

    QJsonArray result = doc["data"].toObject()["result"].toArray();

    if (result.isEmpty())
    {
        qWarning() << "[Prometheus] Empty result array";

        return;
    }

    if (!m_is_single_request)
    {
        for (const auto &entry : std::as_const(result))
        {
            QJsonObject obj = entry.toObject();
            QString name = obj["metric"].toObject().value("__name__").toString();
            double value = obj["value"].toArray()[1].toString().toDouble();

            // qDebug() << "[Prometheus] Got " << name << ": " << value;

            if (name == "node_boot_time_seconds")
            {
                m_values["uptime"] = QDateTime().currentSecsSinceEpoch() - value;
            }
            else if (name == "node_memory_MemAvailable_bytes")
            {
                m_values["free_memory"] = value;
            }
            else if (name == "node_memory_MemTotal_bytes")
            {
                m_values["all_memory"] = value;
            }
            else if (name == "nvidia_gpu_temperature_celsius")
            {
                m_values["gpu_temp"] = value;
            }
            else if (name == "nvidia_gpu_duty_cycle")
            {
                m_values["gpu_usage"] = value;
            }
        }

        m_values["ram_usage"] = (1 - (m_values["free_memory"].toDouble() / m_values["all_memory"].toDouble())) * 100;
    }
    else
    {
        QJsonObject obj = result[0].toObject();
        QJsonArray arr = obj["value"].toArray();
        double value = 0;

        if (arr.size() >= 2)
        {
            value = arr[1].toString().toDouble();
        }

        // qDebug() << "Obj is:" << obj
        //          << ", first key:" << m_queries.firstKey()
        //          << ", value:" << value;

        m_values[m_queries.firstKey()] = value;
    }

    emit metricsChanged();
}
