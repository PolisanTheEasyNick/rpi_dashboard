#ifndef PROMETHEUSCLIENT_H
#define PROMETHEUSCLIENT_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class PrometheusClient :
    public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString endpoint READ endpoint WRITE setEndpoint NOTIFY endpointChanged)
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY metricsChanged)

public:
    explicit PrometheusClient(QObject *parent = nullptr, bool is_single_request = false);
    QString endpoint() const;
    void setEndpoint(const QString &url);
    void setSingleRequest(bool isSingle);

    QVariantMap metrics() const;

    Q_INVOKABLE void addMetric(const QString &key, const QString &query);
    Q_INVOKABLE void update();

signals:
    void endpointChanged();
    void metricsChanged();
    void errorOccured(const QString &msg);

private slots:
    void handleResponse(QNetworkReply *reply);

private:
    QString m_endpoint;
    QVariantMap m_queries;   // key → PromQL expression
    QVariantMap m_values;    // key → value
    QNetworkAccessManager m_mgr;
    // QNetworkReply *m_reply = nullptr;
    bool m_is_single_request = false;
};

#endif // PROMETHEUSCLIENT_H
