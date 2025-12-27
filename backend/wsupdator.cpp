// wsupdator.cpp
#include "wsupdator.h"
#include <QDebug>

WSUpdator::WSUpdator(QObject *parent) :
    QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &WSUpdator::onConnected, Qt::UniqueConnection);
    connect(&m_socket, &QWebSocket::disconnected, this, &WSUpdator::onDisconnected, Qt::UniqueConnection);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WSUpdator::attemptReconnect, Qt::UniqueConnection);

    m_reconnectTimer.setInterval(3000);
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer,
            &QTimer::timeout,
            this,
            &WSUpdator::attemptReconnect,
            Qt::UniqueConnection);
}

QString
WSUpdator::getValue() const
{
    return m_value;
}

void
WSUpdator::setServer(const QUrl &url)
{
    m_server = url;
}

void
WSUpdator::start()
{
    if (m_running
        || !m_server.isValid())
    {
        return;
    }

    qDebug() << "[WSUpdator] Starting:" << m_server;

    m_running = true;
    m_reconnectTimer.stop();
    m_socket.open(m_server);
}

void
WSUpdator::stop()
{
    if (!m_running)
    {
        return;
    }

    qDebug() << "[WSUpdator] Stopping";

    m_running = false;
    m_reconnectTimer.stop();
    m_socket.abort();
}

bool
WSUpdator::isRunning() const
{
    return m_running;
}

void
WSUpdator::onConnected()
{
    qDebug() << "[WSUpdator] Connected";
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &WSUpdator::onTextMessageReceived, Qt::UniqueConnection);
}

void
WSUpdator::onTextMessageReceived(const QString &msg)
{
    if (m_value == msg)
    {
        return;
    }

    m_value = msg;
    emit valueChanged();
}

void
WSUpdator::onDisconnected()
{
    qDebug() << "[WSUpdator] Disconnected";

    if (!m_running)
    {
        return;
    }

    qWarning() << "WebSocket closed. Reconnecting in 10 seconds...";
    m_reconnectTimer.start(10000);
}

void
WSUpdator::attemptReconnect()
{
    if (!m_running)
    {
        return;
    }

    if (!m_server.isEmpty())
    {
        m_socket.open(m_server);
        m_reconnectTimer.stop();
    }
}
