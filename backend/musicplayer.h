#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QDir>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QWebSocket>
#include <QWebSocketServer>
#include <QUrlQuery>

class MusicPlayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY changed)
    Q_PROPERTY(QString artist READ artist NOTIFY changed)
    Q_PROPERTY(QString album READ album NOTIFY changed)
    Q_PROPERTY(QString artURL READ artURL NOTIFY changed)
    Q_PROPERTY(bool musicPlayerStarted READ musicPlayerStarted NOTIFY changed)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY changed)
    Q_PROPERTY(bool gamemodeStarted READ gamemodeStarted NOTIFY changed)

public:
    explicit MusicPlayer(QObject *parent = nullptr);

    QString title() const { return m_title; }
    QString artist() const { return m_artist; }
    QString album() const { return m_album; }
    QString artURL() const { return m_artURL; }
    bool musicPlayerStarted() const { return m_musicPlayerStarted; }
    bool isPlaying() const { return m_isPlaying; }
    bool gamemodeStarted() const { return m_gamemodeStarted; }

    void setSpotifyUrl(const QUrl url, const QUrl album_api)
    {
        m_spotifyUrl = url;
        m_spotifyAlbumUrl = album_api;
    }

    void setNavidromePort(quint16 port)
    {
        m_navidromeWsPort = port;
        startNavidromeServer();
    }

signals:
    void changed();

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onDisconnected();
    void reconnect();
    
    // navidrome server Slots
    void onNavidromeNewConnection();
    void onNavidromeMessageReceived(const QString &message);

private:
    QWebSocket m_ws;
    QTimer m_reconnectTimer;

    QWebSocketServer *m_navidromeServer = nullptr;
    QWebSocket *m_navidromeClient = nullptr;

    QUrl m_spotifyUrl, m_spotifyAlbumUrl;
    quint16 m_navidromeWsPort = 0;

    QString m_title;
    QString m_artist;
    QString m_album;
    QString m_artURL;
    bool m_musicPlayerStarted = false;
    bool m_isPlaying = false;
    bool m_gamemodeStarted = false;

    bool m_spotifyIsPlaying = false;
    bool m_spotifyIsStarted = false;
    QString m_spotifyTitle;
    QString m_spotifyArtist;
    QString m_spotifyAlbum;
    QString m_spotifyArtURL;

    bool m_navidromeIsPlaying = false;
    bool m_navidromeIsStarted = false;
    QString m_navidromeTitle;
    QString m_navidromeArtist;
    QString m_navidromeAlbum;
    QString m_navidromeArtURL;

    QDir m_cacheDir;
    QHash<QString, QString> m_localCache;

    QNetworkAccessManager m_net;
    bool m_fetchInProgress = false;

    void startNavidromeServer();

    QString cacheFilePath(const QString &key, const QString &url);
    void fetchAlbumArtFallback(const QString &title, const QString &artist, const QString &artUrlParam);
    void downloadImage(const QString &key, const QString &url);

    void updateCombinedState();
};

#endif // MUSICPLAYER_H
