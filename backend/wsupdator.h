// wsupdator.h
#ifndef WSUPDATOR_H
#define WSUPDATOR_H

#include <QObject>
#include <QTimer>
#include <QWebSocket>

class WSUpdator :
    public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString value READ getValue NOTIFY valueChanged)

public:
    explicit WSUpdator(QObject *parent = nullptr);

    QString getValue() const;

    Q_INVOKABLE void setServer(const QUrl &url);
    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
    bool isRunning() const;

signals:
    void valueChanged();
    void connectionError(const QString &msg);

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &msg);
    void onDisconnected();
    void attemptReconnect();

private:
    QWebSocket m_socket;
    QUrl m_server;
    QString m_value = 0;
    QTimer m_reconnectTimer;
    bool m_running = false;
};

#endif // WSUPDATOR_H
