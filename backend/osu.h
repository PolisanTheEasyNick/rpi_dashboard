#ifndef OSU_H
#define OSU_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QWebSocket>

class Osu :
    public QObject
{
    Q_OBJECT

    // -- QML Properties --
    Q_PROPERTY(bool playing      READ playing     NOTIFY stateChanged)
    Q_PROPERTY(bool started      READ started     NOTIFY stateChanged)
    Q_PROPERTY(int state         READ state       NOTIFY gameStateChanged)

    // Track Info
    Q_PROPERTY(QString title     READ title       NOTIFY trackChanged)
    Q_PROPERTY(QString artist    READ artist      NOTIFY trackChanged)
    Q_PROPERTY(double difficulty READ difficulty  NOTIFY trackChanged)
    Q_PROPERTY(int bpm           READ bpm         NOTIFY trackChanged)

    // Gameplay Stats
    Q_PROPERTY(int pp            READ pp          NOTIFY statsChanged)
    Q_PROPERTY(int hits100       READ hits100     NOTIFY statsChanged)
    Q_PROPERTY(int hits50        READ hits50      NOTIFY statsChanged)
    Q_PROPERTY(int hitsMiss      READ hitsMiss    NOTIFY statsChanged)
    Q_PROPERTY(QString grade     READ grade       NOTIFY statsChanged)
    Q_PROPERTY(QString mapRank   READ rank        NOTIFY statsChanged)
    Q_PROPERTY(QString rankColor READ rank_color  NOTIFY statsChanged)

public:
    explicit Osu(QObject *parent = nullptr);

    // -- Methods to control connection from Backend --
    void start(); // Connect
    void stop();  // Disconnect/Pause

    // -- Getters --
    bool
    playing() const { return m_osu_playing; }
    bool
    started() const { return m_osu_started; }
    QString
    title() const { return m_title; }
    QString
    artist() const { return m_artist; }
    double
    difficulty() const { return m_difficulty; }
    int
    bpm() const { return m_bpm; }
    int
    pp() const { return m_pp; }
    int
    hits100() const { return m_hits100; }
    int
    hits50() const { return m_hits50; }
    int
    hitsMiss() const { return m_hitsMiss; }
    QString
    grade() const { return m_grade; }
    int
    state() const { return m_state;}

    QString
    rank() const {return m_map_rank;}

    QString
    rank_color() const {return m_rank_color;}

    void
    setUrl(const QString url)
    {
        m_url = url;
        m_fullPath = m_url;
        m_fullPath.setPath(m_fullPath.path() + "/websocket/v2");
    }

signals:
    void stateChanged();
    void trackChanged();
    void statsChanged();
    void gameStateChanged();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void reconnect();
    void onPpReply(QNetworkReply *reply);

private:
    void parsePacket(const QJsonObject &root);
    void fetchMenuPP();

    QWebSocket m_webSocket;
    QTimer m_reconnectTimer;
    QUrl m_url, m_fullPath;
    QNetworkAccessManager *m_nam;

    bool m_osu_started = false;
    bool m_osu_playing = false;

    QString m_title = "Song";
    QString m_artist = "Artist";
    double m_difficulty = 0.0;
    int m_bpm = 0;
    QString m_mods;

    int m_pp = 0;
    int m_hits100 = 0;
    int m_hits50 = 0;
    int m_hitsMiss = 0;
    QString m_grade = "";
    int m_state = 0;
    QString m_map_rank = "", m_rank_color = "";
};

#endif // OSU_H
