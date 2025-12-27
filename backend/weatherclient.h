#ifndef WEATHERCLIENT_H
#define WEATHERCLIENT_H

#include <QObject>
#include <QWebSocket>

class WeatherClient :
    public QObject
{
    Q_OBJECT

    Q_PROPERTY(double temperature READ temperature NOTIFY weatherChanged)
    Q_PROPERTY(QString description READ description NOTIFY weatherChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY weatherChanged)
    Q_PROPERTY(int uvIndex READ uvIndex NOTIFY weatherChanged)
    Q_PROPERTY(bool day READ day NOTIFY weatherChanged FINAL)

public:
    explicit WeatherClient(QObject *parent = nullptr);

    Q_INVOKABLE void setServer(const QString &url);
    Q_INVOKABLE void connectSocket();

    double temperature() const;
    QString description() const;
    QString icon() const;
    int uvIndex() const;
    bool day() const;

signals:
    void weatherChanged();
    void connectionError(const QString &msg);

private slots:
    void onMessageReceived(const QString &msg);
    void onClosed();

private:
    QWebSocket m_socket;
    QString m_server;
    double m_temperature = 0;
    QString m_description;
    QString m_icon;
    int m_uvIndex = 0;
    bool m_isDay = true;
};

#endif // WEATHERCLIENT_H
