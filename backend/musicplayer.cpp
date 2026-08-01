#include "musicplayer.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QStandardPaths>

MusicPlayer::MusicPlayer(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[MusicPlayer] Constructor";
    
    // spotify ws
    connect(&m_ws, &QWebSocket::connected, this, &MusicPlayer::onConnected, Qt::UniqueConnection);
    connect(&m_ws, &QWebSocket::disconnected, this, &MusicPlayer::onDisconnected, Qt::UniqueConnection);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &MusicPlayer::reconnect, Qt::UniqueConnection);
    m_reconnectTimer.setInterval(5000);
    m_reconnectTimer.start();

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/spotify";
    QDir().mkpath(cacheDir);
    m_cacheDir = cacheDir;
}

void MusicPlayer::startNavidromeServer()
{
    if (m_navidromeServer) {
        m_navidromeServer->close();
        m_navidromeServer->deleteLater();
    }

    m_navidromeServer = new QWebSocketServer(QStringLiteral("Navidrome Server"), QWebSocketServer::NonSecureMode, this);
    if (m_navidromeServer->listen(QHostAddress::Any, m_navidromeWsPort)) {
        qDebug() << "[MusicPlayer] Navidrome WebSocket server listening on port" << m_navidromeWsPort;
        connect(m_navidromeServer, &QWebSocketServer::newConnection, this, &MusicPlayer::onNavidromeNewConnection);
    } else {
        qWarning() << "[MusicPlayer] Failed to start Navidrome WebSocket server on port" << m_navidromeWsPort;
    }
}

void MusicPlayer::onNavidromeNewConnection()
{
    qDebug() << "[MusicPlayer] New Navidrome client connected";
    QWebSocket *pSocket = m_navidromeServer->nextPendingConnection();
    
    // oupport one client at a time for simplicity
    if (m_navidromeClient) {
        m_navidromeClient->close();
        m_navidromeClient->deleteLater();
    }
    m_navidromeClient = pSocket;

    connect(pSocket, &QWebSocket::textMessageReceived, this, &MusicPlayer::onNavidromeMessageReceived);
    connect(pSocket, &QWebSocket::disconnected, this, [this, pSocket]() {
        qDebug() << "[MusicPlayer] Navidrome client disconnected";
        if (m_navidromeClient == pSocket) {
            m_navidromeClient = nullptr;
        }
        pSocket->deleteLater();
    });
}

void MusicPlayer::onNavidromeMessageReceived(const QString &message)
{
    qDebug() << "[MusicPlayer] Raw Navidrome message:" << message;
    
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[MusicPlayer] Navidrome JSON parse error:" << err.errorString();
        return;
    }

    QJsonObject obj = doc.object();
    
    m_navidromeTitle = obj.value("title").toString();
    m_navidromeArtist = obj.value("artist").toString();
    m_navidromeAlbum = obj.value("album").toString();
    m_navidromeArtURL = obj.value("artURL").toString();
    
    m_navidromeIsStarted = obj.value("navidromeStarted").toBool();
    m_navidromeIsPlaying = obj.value("isPlaying").toBool();
    
    qDebug() << "[MusicPlayer] Navidrome push received: playing=" << m_navidromeIsPlaying << "title=" << m_navidromeTitle;
    
    updateCombinedState();
}

void MusicPlayer::onConnected()
{
    qDebug() << "[MusicPlayer] Connected to Spotify WS:" << m_spotifyUrl;
    connect(&m_ws, &QWebSocket::textMessageReceived, this, &MusicPlayer::onTextMessageReceived, Qt::UniqueConnection);
    m_reconnectTimer.stop();
}

void MusicPlayer::onDisconnected()
{
    qDebug() << "[MusicPlayer] Spotify Disconnected. Reconnecting...";
    m_reconnectTimer.start();
    m_spotifyIsPlaying = false;
    updateCombinedState();
}

void MusicPlayer::reconnect()
{
    if (m_spotifyUrl.isEmpty()) return;
    qDebug() << "[MusicPlayer] Reconnecting to Spotify WS:" << m_spotifyUrl;

    if (m_ws.state() != QAbstractSocket::ConnectedState)
    {
        m_ws.open(m_spotifyUrl);
    }
    m_spotifyIsPlaying = false;
    updateCombinedState();
}

void MusicPlayer::updateCombinedState()
{
    bool changedFlag = false;

    auto updateString = [&](QString &field, const QString &value) {
        if (field != value) {
            field = value;
            return true;
        }
        return false;
    };
    auto updateBool = [&](bool &field, bool value) {
        if (field != value) {
            field = value;
            return true;
        }
        return false;
    };

    bool newIsPlaying = m_navidromeIsPlaying || m_spotifyIsPlaying;
    bool newIsStarted = m_navidromeIsStarted || m_spotifyIsStarted;
    
    QString newTitle, newArtist, newAlbum, newArtURL;
    
    if (m_navidromeIsPlaying) {
        newTitle = m_navidromeTitle;
        newArtist = m_navidromeArtist;
        newAlbum = m_navidromeAlbum;
        newArtURL = m_navidromeArtURL;
    } else if (m_spotifyIsPlaying) {
        newTitle = m_spotifyTitle;
        newArtist = m_spotifyArtist;
        newAlbum = m_spotifyAlbum;
        newArtURL = m_spotifyArtURL;
    } else {
        if (m_navidromeIsStarted && !m_spotifyIsStarted) {
            newTitle = m_navidromeTitle;
            newArtist = m_navidromeArtist;
            newAlbum = m_navidromeAlbum;
            newArtURL = m_navidromeArtURL;
        } else {
            newTitle = m_spotifyTitle;
            newArtist = m_spotifyArtist;
            newAlbum = m_spotifyAlbum;
            newArtURL = m_spotifyArtURL;
        }
    }

    changedFlag |= updateBool(m_isPlaying, newIsPlaying);
    changedFlag |= updateBool(m_musicPlayerStarted, newIsStarted);
    changedFlag |= updateString(m_title, newTitle);
    changedFlag |= updateString(m_artist, newArtist);
    changedFlag |= updateString(m_album, newAlbum);
    changedFlag |= updateString(m_artURL, newArtURL);

    if (changedFlag) {
        emit changed();
    }
}

void MusicPlayer::fetchAlbumArtFallback(const QString &title, const QString &artist, const QString &artUrlParam)
{
    if (m_fetchInProgress || title.isEmpty()) {
        return;
    }

    m_fetchInProgress = true;
    QString key = title;
    if (!artist.isEmpty()) {
        key += "-" + artist;
    }

    qDebug() << "[MusicPlayer] fetchAlbumArtFallback for key:" << key;

    if (m_localCache.contains(key) && QFile::exists(m_localCache[key])) {
        m_spotifyArtURL = "file://" + m_localCache[key];
        updateCombinedState();
        m_fetchInProgress = false;
        return;
    }

    QUrl url(m_spotifyAlbumUrl);
    QUrlQuery q;
    q.addQueryItem("song", key);
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "rpi-dashboard");

    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, key]() {
        m_fetchInProgress = false;

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[MusicPlayer] Album art API request failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
        reply->deleteLater();

        if (err.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "[MusicPlayer] Invalid JSON from album API:" << err.errorString();
            return;
        }

        QString newUrl = doc.object().value("album_art_url").toString();
        if (newUrl.isEmpty()) {
            qWarning() << "[MusicPlayer] album_art_url missing in API response";
            return;
        }

        downloadImage(key, newUrl);
    });
}

void MusicPlayer::downloadImage(const QString &key, const QString &url)
{
    qDebug() << "[MusicPlayer] downloadImage called. Key:" << key << "URL:" << url;

    QString localFile = cacheFilePath(key, url);
    if (QFile::exists(localFile)) {
        qDebug() << "[MusicPlayer] Image already cached:" << localFile;
        m_localCache[key] = localFile;
        m_spotifyArtURL = "file://" + localFile;
        updateCombinedState();
        return;
    }

    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::UserAgentHeader, "rpi-dashboard");

    QNetworkReply *reply = m_net.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, key, localFile]() {
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[MusicPlayer] Image download failed:" << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray imgData = reply->readAll();
        reply->deleteLater();

        QFile f(localFile);
        if (!f.open(QIODevice::WriteOnly)) {
            qWarning() << "[MusicPlayer] Failed to write file:" << localFile;
            return;
        }

        f.write(imgData);
        f.close();

        m_localCache[key] = localFile;
        m_spotifyArtURL = "file://" + localFile;
        qDebug() << "[MusicPlayer] Image cached and ready:" << m_spotifyArtURL;
        updateCombinedState();
    });
}

void MusicPlayer::onTextMessageReceived(const QString &message)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "[MusicPlayer] JSON parse error:" << err.errorString();
        return;
    }

    QJsonObject obj = doc.object();
    
    m_spotifyTitle = obj.value("title").toString();
    m_spotifyArtist = obj.value("artist").toString();
    m_spotifyAlbum = obj.value("album").toString();
    QString artURL = obj.value("artURL").toString();
    
    m_spotifyIsStarted = (obj.value("spotifyStarted").toString().compare("True", Qt::CaseInsensitive) == 0);
    m_spotifyIsPlaying = (obj.value("isPlaying").toString().compare("True", Qt::CaseInsensitive) == 0);
    
    bool gmStarted = (obj.value("gamemodeStarted").toString().compare("True", Qt::CaseInsensitive) == 0);
    if (m_gamemodeStarted != gmStarted) {
        m_gamemodeStarted = gmStarted;
        emit changed();
    }

    if (artURL.isEmpty()) {
        QString key = m_spotifyTitle + "-" + m_spotifyArtist;
        QString localFile = cacheFilePath(key, artURL);

        if (QFile::exists(localFile)) {
            m_spotifyArtURL = "file://" + localFile;
        } else {
            fetchAlbumArtFallback(m_spotifyTitle, m_spotifyArtist, artURL);
        }
    } else {
        QString key = QUrl(artURL).fileName();
        downloadImage(key, artURL);
    }

    updateCombinedState();
}

QString MusicPlayer::cacheFilePath(const QString &key, const QString &url)
{
    QString safeKey = key;
    safeKey.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
    QString ext = QFileInfo(QUrl(url).path()).suffix();
    if (ext.isEmpty()) {
        ext = "jpg";
    }
    return m_cacheDir.path() + "/" + safeKey + "." + ext;
}
