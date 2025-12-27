#include "osu.h"
#include <QDebug>
#include <QUrlQuery>

Osu::Osu(QObject *parent) :
    QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);

    connect(&m_webSocket, &QWebSocket::connected, this, &Osu::onConnected, Qt::UniqueConnection);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Osu::onDisconnected, Qt::UniqueConnection);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &Osu::onTextMessageReceived, Qt::UniqueConnection);

    m_reconnectTimer.setInterval(5000);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &Osu::reconnect, Qt::UniqueConnection);
}

void
Osu::start()
{
    qDebug() << "[Osu] m_url: " << m_fullPath << "; is empty: " << m_fullPath.isEmpty();

    if (!m_fullPath.isEmpty()
        || m_webSocket.state() == QAbstractSocket::UnconnectedState)
    {
        qDebug() << "[Osu] Connecting to" << m_fullPath.path();
        m_webSocket.open(m_fullPath);
    }
    else if (m_url.isEmpty())
    {
        qDebug() << "[Osu] Url is empty, starting reconnect timer.";
        m_reconnectTimer.start();
    }
}

void
Osu::stop()
{
    m_reconnectTimer.stop();
    m_webSocket.close();
}

void
Osu::reconnect()
{
    qDebug() << "[Osu] ReConnecting to" << m_fullPath;
    m_webSocket.abort();
    m_webSocket.open(m_fullPath);
}

void
Osu::onPpReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);

        if (!doc.isNull()
            && doc.isObject())
        {
            int newPp = doc["pp"].toDouble();

            if (m_pp != newPp)
            {
                m_pp = newPp;
                emit statsChanged();
                qDebug() << "[Osu] Got PP response: " << m_pp;
            }
        }
    }
    else
    {
        qDebug() << "[Osu] PP Calc Error:" << reply->errorString();
    }

    reply->deleteLater();
}

void
Osu::onConnected()
{
    qDebug() << "[Osu] Connected!";
    m_reconnectTimer.stop();
}

void
Osu::onDisconnected()
{
    qDebug() << "[Osu] Disconnected. Retrying...";
    m_osu_playing = false;
    m_osu_started = false;
    emit stateChanged();

    m_reconnectTimer.start();
}

void
Osu::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    if (!doc.isNull()
        && doc.isObject())
    {
        parsePacket(doc.object());
    }
}

void
Osu::parsePacket(const QJsonObject &root)
{
    // qDebug() << "Parsing root: " << root;
    // --- 1. Parse Game State ---
    // State Codes: 2=Playing, 7=Results, 11/14=Multiplayer
    int gameState = root["state"].toObject()["number"].toInt();

    if (gameState == 3) // exit
    {
        m_osu_started = false;
        emit stateChanged();

        return;
    }
    else if (!m_osu_started)
    {
        m_osu_started = true;
        emit stateChanged();
    }

    short int old_state = m_state;

    if (m_state != gameState)
    {
        m_state = gameState;
        emit gameStateChanged();
    }

    // --- 2. Parse Metadata (Song Info) ---
    // We parse this regardless of playing state so the menu updates immediately
    QJsonObject beatmap = root["beatmap"].toObject();
    QJsonObject stats = beatmap["stats"].toObject();
    QJsonObject path = beatmap["path"].toObject();

    QString newTitle = beatmap["title"].toString();
    QString newArtist = beatmap["artist"].toString();
    double newDiff = stats["stars"].toObject()["total"].toDouble();
    int newBpm = stats["bpm"].toObject()["max"].toInt();
    int map_rank = beatmap["status"].toObject()["number"].toInt();
    QString newRankColor = "";
    QString newRank = "";

    if (map_rank ==  7)
    {
        // newRank = "";
        newRank = "heart";
        newRankColor = "#ff81c5";
    }
    else if (map_rank ==  4)
    {
        // newRank = "4";
        newRank = "angles-up";
        newRankColor = "#80e6ff";
    }
    else if (map_rank ==  5)
    {
        // newRank = "";
        newRank = "check";
        newRankColor = "#c0e71b";
    }
    else
    {
        // newRank = "";
        newRank = "question";
        newRankColor = "#929292";
    }

    bool trackDiffers = false;

    if (m_map_rank != newRank)
    {
        m_map_rank = newRank;
        m_rank_color = newRankColor;
        trackDiffers = true;
    }

    if (m_title != newTitle)
    {
        m_title = newTitle;
        trackDiffers = true;
    }

    if (m_artist != newArtist)
    {
        m_artist = newArtist;
        trackDiffers = true;
    }

    if (!qFuzzyCompare(m_difficulty, newDiff))
    {
        m_difficulty = newDiff;
        trackDiffers = true;
    }

    if (m_bpm != newBpm)
    {
        m_bpm = newBpm;
        trackDiffers = true;
    }

    if (trackDiffers)
    {
        emit trackChanged();
    }

    // --- 3. Parse Gameplay Stats ---
    // Only strictly necessary if we are active, but parsing it always allows "Results" screen to work
    QJsonObject gameplay = root["play"].toObject();
    QJsonObject hits = gameplay["hits"].toObject();

    bool statsDiffer = false;

    // --- PP Logic (The update you asked for) ---
    bool isActive = (gameState == 2
                     || gameState == 7
                     || gameState == 14);

    if (isActive)
    {
        // CASE A: PLAYING
        // Parse PP from WebSocket stream
        int newPp = gameplay["pp"].toObject()["current"].toDouble();

        if (m_pp != newPp)
        {
            m_pp = newPp;
            statsDiffer = true;
        }
    }
    else
    {
        // CASE B: MENU
        // If Track changed, fetch new PP from API.
        if (trackDiffers
            || old_state != m_state)
        {
            qDebug() << "[Osu] Calling menu pp fetching";
            fetchMenuPP();
        }
    }

    int new100 = hits["100"].toInt();
    int new50 = hits["50"].toInt();
    int newMiss = hits["0"].toInt();
    QString newGrade = gameplay["rank"].toObject()["current"].toString();

    // Map 'X' to 'SS' for consistency if needed
    if (newGrade == "X")
    {
        newGrade = "SS";
    }

    if (newGrade == "XH")
    {
        newGrade = "SS";
    }

    if (m_hits100 != new100)
    {
        m_hits100 = new100;
        statsDiffer = true;
    }

    if (m_hits50 != new50)
    {
        m_hits50 = new50;
        statsDiffer = true;
    }

    if (m_hitsMiss != newMiss)
    {
        m_hitsMiss = newMiss;
        statsDiffer = true;
    }

    if (m_grade != newGrade)
    {
        m_grade = newGrade;
        statsDiffer = true;
    }

    if (statsDiffer)
    {
        emit statsChanged();
        qDebug() << "[Osu] Info: " << "Title: " << m_title << "; Artist: " << m_artist << ", Difficulty: " <<
            m_difficulty << "; BPM: " << m_bpm << "PP: " << m_pp;
    }
}

void
Osu::fetchMenuPP()
{
    QUrl url = m_url;
    url.setPath(url.path() + "/api/calculate/pp");
    QNetworkRequest req(url);
    QNetworkReply *reply = m_nam->get(req);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply]()
            {
                onPpReply(reply);
            }
            );
}
