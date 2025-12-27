#include "spotify.h"
#include <QDebug>

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrlQuery>

Spotify::Spotify(QObject *parent) :
    QObject(parent)
{
    qDebug() << "[Spotify] Constructor";
    connect(&m_ws, &QWebSocket::connected, this, &Spotify::onConnected, Qt::UniqueConnection);
    connect(&m_ws, &QWebSocket::disconnected, this, &Spotify::onDisconnected, Qt::UniqueConnection);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &Spotify::reconnect, Qt::UniqueConnection);
    m_reconnectTimer.setInterval(5000);

    m_reconnectTimer.start();

    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/spotify";
    QDir().mkpath(cacheDir);
    m_cacheDir = cacheDir;
}

void
Spotify::onConnected()
{
    qDebug() << "[Spotify] Connected to" << m_url;
    connect(&m_ws, &QWebSocket::textMessageReceived, this, &Spotify::onTextMessageReceived, Qt::UniqueConnection);
    m_reconnectTimer.stop();
}

void
Spotify::onDisconnected()
{
    qDebug() << "[Spotify] Disconnected. Reconnecting...";
    m_reconnectTimer.start();
}

void
Spotify::reconnect()
{
    qDebug() << "[Spotify] Disconnected. Reconnecting to " << m_url;

    if (m_ws.state() != QAbstractSocket::ConnectedState)
    {
        m_ws.open(m_url);
    }
}

void
Spotify::fetchAlbumArtFallback()
{
    if (m_fetchInProgress
        || m_title.isEmpty())
    {
        return;
    }

    m_fetchInProgress = true;
    QString key = m_title;

    if (!m_artist.isEmpty())
    {
        key += "-" + m_artist;
    }

    qDebug() << "[Spotify] fetchAlbumArtFallback for key:" << key;

    // check local cache first
    if (m_localCache.contains(key)
        && QFile::exists(m_localCache[key]))
    {
        m_artURL = "file://" + m_localCache[key];
        emit changed();
        m_fetchInProgress = false;

        return;
    }

    // fetch album URL from API
    QUrl url(m_album_url);
    QUrlQuery q;
    q.addQueryItem("song", key);
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "rpi-dashboard");

    QNetworkReply *reply = m_net.get(req);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, key]()
            {
                m_fetchInProgress = false;

                if (reply->error() != QNetworkReply::NoError)
                {
                    qWarning() << "[Spotify] Album art API request failed:" << reply->errorString();
                    reply->deleteLater();

                    return;
                }

                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
                reply->deleteLater();

                if (err.error != QJsonParseError::NoError
                    || !doc.isObject())
                {
                    qWarning() << "[Spotify] Invalid JSON from album API:" << err.errorString();

                    return;
                }

                QString newUrl = doc.object().value("album_art_url").toString();

                if (newUrl.isEmpty())
                {
                    qWarning() << "[Spotify] album_art_url missing in API response";

                    return;
                }

                // call generic downloader
                downloadImage(key, newUrl);
            }
            );
}

void
Spotify::downloadImage(const QString &key, const QString &url)
{
    qDebug() << "[Spotify] downloadImage called. Key:" << key << "URL:" << url;

    // Check cache first
    QString localFile = cacheFilePath(key, url);

    if (QFile::exists(localFile))
    {
        qDebug() << "[Spotify] Image already cached:" << localFile;
        m_localCache[key] = localFile;
        m_artURL = "file://" + localFile;
        emit changed();

        return;
    }

    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::UserAgentHeader, "rpi-dashboard");

    QNetworkReply *reply = m_net.get(req);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, key, localFile]()
            {
                if (reply->error() != QNetworkReply::NoError)
                {
                    qWarning() << "[Spotify] Image download failed:" << reply->errorString();
                    reply->deleteLater();

                    return;
                }

                QByteArray imgData = reply->readAll();
                reply->deleteLater();

                QFile f(localFile);

                if (!f.open(QIODevice::WriteOnly))
                {
                    qWarning() << "[Spotify] Failed to write file:" << localFile;

                    return;
                }

                f.write(imgData);
                f.close();

                m_localCache[key] = localFile;
                m_artURL = "file://" + localFile;
                qDebug() << "[Spotify] Image cached and ready:" << m_artURL;
                emit changed();
            }
            );
}

void
Spotify::onTextMessageReceived(const QString &message)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &err);

    if (err.error != QJsonParseError::NoError)
    {
        qWarning() << "[Spotify] JSON parse error:" << err.errorString();

        return;
    }

    if (!doc.isObject())
    {
        return;
    }

    QJsonObject obj = doc.object();
    bool info_changed = false;

    auto updateString = [&](QString &field, const QString &value)
                        {
                            if (field != value)
                            {
                                field = value;

                                return true;
                            }

                            return false;
                        };
    auto updateBool = [&](bool &field, const QString &valueStr)
                      {
                          bool value = (valueStr.compare("True", Qt::CaseInsensitive) == 0);

                          if (field != value)
                          {
                              field = value;

                              return true;
                          }

                          return false;
                      };

    info_changed |= updateString(m_title, obj.value("title").toString());
    info_changed |= updateString(m_artist, obj.value("artist").toString());
    info_changed |= updateString(m_album, obj.value("album").toString());
    info_changed |= updateString(m_artURL, obj.value("artURL").toString());
    info_changed |= updateBool(m_spotifyStarted, obj.value("spotifyStarted").toString());
    info_changed |= updateBool(m_isPlaying, obj.value("isPlaying").toString());
    info_changed |= updateBool(m_gamemodeStarted, obj.value("gamemodeStarted").toString());

    if (m_artURL.isEmpty())
    {
        QString key = m_title + "-" + m_artist;
        QString localFile = cacheFilePath(key, m_artURL);

        if (QFile::exists(localFile))
        {
            m_artURL = "file://" + localFile;
            qDebug() << "[Spotify] Using cached file:" << localFile;
        }
        else
        {
            qDebug() << "[Spotify] Cached file does not exist yet:" << localFile;
            fetchAlbumArtFallback();
        }
    }
    else
    {
        // means song from spotify - use caching too
        QString key = QUrl(m_artURL).fileName();
        downloadImage(key, m_artURL);
    }

    if (info_changed)
    {
        qDebug() << "[Spotify] Info changed, new info: Title: " << m_title << ", Artist: " << m_artist << ", Album: " <<
            m_album << ", ArtURL: " << m_artURL << "Spotify started: " << m_spotifyStarted << ", Spotify playing: " <<
            m_isPlaying << "Gamemode started: " << m_gamemodeStarted;
        emit changed();
    }
}

QString
Spotify::cacheFilePath(const QString &key, const QString &url)
{
    QString safeKey = key;
    safeKey.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_"); // sanitize
    QString ext = QFileInfo(QUrl(url).path()).suffix();

    if (ext.isEmpty())
    {
        ext = "jpg";
    }

    return m_cacheDir.path() + "/" + safeKey + "." + ext;
}
