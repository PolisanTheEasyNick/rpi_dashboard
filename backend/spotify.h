#ifndef SPOTIFY_H
#define SPOTIFY_H

#include <QDBusInterface>
#include <QObject>

#include <QDir>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QWebSocket>

class Spotify :
    public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(QString artist READ artist NOTIFY changed)
    Q_PROPERTY(QString album READ album NOTIFY changed)
    Q_PROPERTY(QString artURL READ artURL NOTIFY changed)
    Q_PROPERTY(bool spotifyStarted READ spotifyStarted NOTIFY changed)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY changed)
    Q_PROPERTY(bool gamemodeStarted READ gamemodeStarted NOTIFY changed)

public:
    explicit Spotify(QObject *parent = nullptr);

    QString
    title() const { return m_title; }
    QString
    artist() const { return m_artist; }
    QString
    album() const { return m_album; }
    QString
    artURL() const { return m_artURL; }
    bool
    spotifyStarted() const { return m_spotifyStarted; }
    bool
    isPlaying() const { return m_isPlaying; }
    bool
    gamemodeStarted() const { return m_gamemodeStarted; }

    void
    setUrl(const QUrl url, const QUrl album_api)
    {
        m_url = url;
        m_album_url = album_api;
    }

signals:
    void changed();

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onDisconnected();
    void reconnect();

private:
    QWebSocket m_ws;
    QTimer m_reconnectTimer;
    QUrl m_url, m_album_url;

    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_artURL;
    bool m_spotifyStarted = false;
    bool m_isPlaying = false;
    bool m_gamemodeStarted = false;
    QDir m_cacheDir;
    QHash<QString, QString> m_localCache;

    QString cacheFilePath(const QString &key, const QString &url);

    QNetworkAccessManager m_net;
    bool m_fetchInProgress = false;

    void fetchAlbumArtFallback();
    void downloadImage(const QString &key, const QString &url);
};

#endif // SPOTIFY_H
