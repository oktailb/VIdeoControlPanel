#include "cpanelmainwindow.h"
#include "./ui_cpanelmainwindow.h"
#include <QDebug>
#include <QGridLayout>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimediaWidgets/QtMultimediaWidgets>
#include <QFileSystemWatcher>
//#include <QtOpenGLWidgets/QtOpenGLWidgets> // Ajout de l'inclusion QOpenGLWidget
#include "vlcmediaplayer.h"

void CPanelMainWindow::playSynchronized(QMediaPlayer *player, int delay)
{
    QTimer::singleShot(delay, [player] {
        player->play();
    });
}

void CPanelMainWindow::resyncPlayers()
{
    if (players.size() < 2) return;
    qint64 basePosition = players.first()->position();

    foreach(auto player, players)
    {
        qint64 currentPosition = player->position();
        qint64 positionDifference = qAbs(basePosition - currentPosition);

        if (positionDifference > resyncThreshold)
        {
            player->setPosition(basePosition);
        }
    }
}

CPanelMainWindow::CPanelMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CPanelMainWindow)
{
    ui->setupUi(this);
    readConfiguration();

    watcher = new QFileSystemWatcher({conf});

    connect(watcher, &QFileSystemWatcher::fileChanged,
            this, &CPanelMainWindow::onConfigChanged);


    QStringList streamList = configuration["displayOrder"].split(':');
    resyncThreshold = configuration["resyncThreshold"].toInt();
    resyncInterval = configuration["resyncInterval"].toInt();
    QGraphicsScene *scene_fg = new QGraphicsScene(  configuration["x"].toFloat(), configuration["y"].toFloat(),
                                                    configuration["width"].toFloat(), configuration["height"].toFloat(),
                                                    this);
    ui->graphicsView_fg->setScene(scene_fg);

    if (configuration.find("overlay") == configuration.end())
        ui->graphicsView_fg->setStyleSheet("background-image: url(:/pictures/overlay.png);");
    else
        ui->graphicsView_fg->setStyleSheet("background-image: url(" + configuration["overlay"] + ");");

    QGraphicsScene *scene_bg = new QGraphicsScene(  configuration["x"].toFloat(), configuration["y"].toFloat(),
                                                    configuration["width"].toFloat(), configuration["height"].toFloat(),
                                                    this);
    ui->graphicsView_bg->setScene(scene_bg);
    setWindowTitle("CPanel");

    connect(&m_resyncTimer, &QTimer::timeout, this, &CPanelMainWindow::resyncPlayers);
    m_resyncTimer.start(resyncInterval);

    m_timer.start();

    for (int streamNum = 0 ; streamNum < streamList.count() ; streamNum++)
    {
        QString name        = streamList[streamNum];
        QString liveUrlStr  = configuration[name + "/liveUrl"];
        int     x           = configuration[name + "/x"].toInt();
        int     y           = configuration[name + "/y"].toInt();
        int     width       = configuration[name + "/width"].toInt();
        int     height      = configuration[name + "/height"].toInt();
        int     rotate      = configuration[name + "/rotate"].toInt();
        int     ox           = -1;
        int     oy           = -1;
        int     owidth       = -1;
        int     oheight      = -1;
        if (configuration.find(name + "/ox") != configuration.end())
            ox           = configuration[name + "/ox"].toInt();
        if (configuration.find(name + "/oy") != configuration.end())
            oy           = configuration[name + "/oy"].toInt();
        if (configuration.find(name + "/owidth") != configuration.end())
            owidth       = configuration[name + "/owidth"].toInt();
        if (configuration.find(name + "/oheight") != configuration.end())
            oheight      = configuration[name + "/oheight"].toInt();
        QString layer       = configuration[name + "/layer"];

        QVideoWidget *          glWidget = new QVideoWidget; // Remplacement de QVideoWidget par QOpenGLWidget
        glWidget->setStyleSheet("background-color: black");
        QMediaPlayer *          player;

        if (configuration["useVLC"].compare("true") == 0) {
            player   = new VlcMediaPlayer(configuration["networkCaching"], configuration["hwDecoding"].compare("true")==0?true:false);
            libvlc_media_player_t *vlcPlayer = static_cast<VlcMediaPlayer*>(player)->core();
            libvlc_video_set_deinterlace(vlcPlayer, "yadif");
            if ((ox != -1) && (oy != -1) && (owidth != -1) && (oheight != -1))
            {
                QString cropGeometry = QString("%1x%2+%3+%4").arg(owidth).arg(oheight).arg(ox).arg(oy);
                libvlc_video_set_crop_geometry(vlcPlayer, cropGeometry.toUtf8().constData());
            }
        }
        else
            player   = new QMediaPlayer();

        //glWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);
//        glWidget->setGraphicsEffect(new QGraphicsBlurEffect(player));

        QGraphicsProxyWidget *  proxyWidget = ((layer.compare("bg") == 0) ? scene_bg : scene_fg)->addWidget(glWidget);
        const QUrl              url         = QUrl(liveUrlStr);

        proxyWidget->setRotation(rotate);
        proxyWidget->setGeometry(QRect(x, y, width, height));
        player->setVideoOutput(glWidget);

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
        player->setSource(url);
#else
        player->setMedia(url);
#endif
        proxies[name] = proxyWidget;
        players[name] = player;
        glWidget->show();

        playSynchronized(player, m_timer.elapsed());
        //player->play();
    }
    if (configuration["stream"].compare("true") == 0)
        startRtspStream();
}

void CPanelMainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)
    {
        stop();
        exit(0);
    }
}

void CPanelMainWindow::onConfigChanged(const QString &path)
{
    qDebug() << "INFO: = " << path;
    readConfiguration();

    QStringList streamList = configuration["displayOrder"].split(':');

    for (int streamNum = 0 ; streamNum < streamList.count() ; streamNum++)
    {
        QString name        = streamList[streamNum];
        QString liveUrlStr  = configuration[name + "/liveUrl"];
        int     x           = configuration[name + "/x"].toInt();
        int     y           = configuration[name + "/y"].toInt();
        int     width       = configuration[name + "/width"].toInt();
        int     height      = configuration[name + "/height"].toInt();
        int     rotate      = configuration[name + "/rotate"].toInt();
        int     ox           = -1;
        int     oy           = -1;
        int     owidth       = -1;
        int     oheight      = -1;
        if (configuration.find(name + "/ox") != configuration.end())
            ox           = configuration[name + "/ox"].toInt();
        if (configuration.find(name + "/oy") != configuration.end())
            oy           = configuration[name + "/oy"].toInt();
        if (configuration.find(name + "/owidth") != configuration.end())
            owidth       = configuration[name + "/owidth"].toInt();
        if (configuration.find(name + "/oheight") != configuration.end())
            oheight      = configuration[name + "/oheight"].toInt();
        QString layer       = configuration[name + "layer"];
        QMediaPlayer *          player = players[name];

        if (configuration["useVLC"].compare("true") == 0) {
            libvlc_media_player_t *vlcPlayer = static_cast<VlcMediaPlayer*>(player)->core();
            libvlc_video_set_deinterlace(vlcPlayer, "yadif");
            if ((ox != -1) && (oy != -1) && (owidth != -1) && (oheight != -1))
            {
                QString cropGeometry = QString("%1x%2+%3+%4").arg(owidth).arg(oheight).arg(ox).arg(oy);
                libvlc_video_set_crop_geometry(vlcPlayer, cropGeometry.toUtf8().constData());
            }
        }

        QGraphicsProxyWidget *  proxyWidget = proxies[name];

        proxyWidget->setRotation(rotate);
        proxyWidget->setGeometry(QRect(x, y, width, height));
    }


}

void CPanelMainWindow::readConfiguration()
{
    try {
        QSettings settings(conf, QSettings::IniFormat);
        const QStringList keys = settings.allKeys();
        foreach (const QString &key, keys)
        {
            configuration.insert(key, settings.value(key).toString());
        }
    } catch (...) {
        //exit(-1);
    }
}

void CPanelMainWindow::stop()
{
    foreach(auto player, players)
    {
        player->stop();
        delete player;
    }
}

CPanelMainWindow::~CPanelMainWindow()
{
    stop();
    delete ui;
    libvlc_media_player_stop(m_vlcMediaPlayer);
    libvlc_media_player_release(m_vlcMediaPlayer);
    libvlc_release(m_vlcInstance);
}

QImage CPanelMainWindow::captureWindow()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen)
    {
        qDebug() << "Failed to get primary screen for capturing.";
        return QImage();
    }

    QPixmap pixmap = screen->grabWindow(this->winId());
    return pixmap.toImage();
}

int CPanelMainWindow::openMediaCb(void *opaque, void **datap, uint64_t *sizep)
{
    CPanelMainWindow *window = static_cast<CPanelMainWindow*>(opaque);
    QImage image = window->captureWindow();
    QByteArray *buffer = new QByteArray();
    QBuffer qbuffer(buffer);
    qbuffer.open(QIODevice::WriteOnly);
    image.save(&qbuffer, "JPEG");
    qbuffer.close();

    *datap = buffer;
    *sizep = buffer->size();
    return 0;
}

ssize_t CPanelMainWindow::readMediaCb(void *opaque, unsigned char *buf, size_t len)
{
    QByteArray *buffer = static_cast<QByteArray*>(opaque);

    if (buffer->size() == 0)
    {
        return 0;
    }

    size_t bytesToRead = std::min<size_t>(len, buffer->size());
    memcpy(buf, buffer->data(), bytesToRead);
    buffer->remove(0, bytesToRead);

    return bytesToRead;
}

int CPanelMainWindow::seekMediaCb(void *opaque, uint64_t offset)
{
    QByteArray *buffer = static_cast<QByteArray*>(opaque);
    buffer->remove(0, static_cast<int>(offset));
    return 0;
}

void CPanelMainWindow::closeMediaCb(void *opaque)
{
    QByteArray *buffer = static_cast<QByteArray*>(opaque);
    delete buffer;
}

void CPanelMainWindow::startRtspStream()
{
    QString rtspPort = configuration["rtspPort"];
    QString rtspPath = configuration["rtspPath"];
    QString networkInterface = configuration["networkInterface"];
    QString rtspSout = QString("#transcode{vcodec=h264,vb=2000,acodec=none}:rtp{sdp=rtsp://%1:%2%3}")
            .arg(networkInterface).arg(rtspPort).arg(rtspPath);

    const char *vlcArgs[] = {
        "--no-xlib",
        "--sout", rtspSout.toUtf8().constData(),
        "--sout-keep"
    };

    libvlc_instance_t *vlcInstance = libvlc_new(sizeof(vlcArgs) / sizeof(vlcArgs[0]), vlcArgs);
    if (!vlcInstance)
    {
        qDebug() << "Failed to create libVLC instance.";
        return;
    }

    libvlc_media_t *media = libvlc_media_new_callbacks(vlcInstance,
                                                        &CPanelMainWindow::openMediaCb,
                                                        &CPanelMainWindow::readMediaCb,
                                                        &CPanelMainWindow::seekMediaCb,
                                                        &CPanelMainWindow::closeMediaCb,
                                                        this);
    if (!media)
    {
        qDebug() << "Failed to create libVLC media with callbacks.";
        return;
    }

    libvlc_media_player_t *mediaPlayer = libvlc_media_player_new_from_media(media);
    if (!mediaPlayer)
    {
        qDebug() << "Failed to create libVLC media player.";
        return;
    }

    libvlc_media_release(media);

    libvlc_media_player_play(mediaPlayer);

    // Store the libVLC objects for later cleanup
    m_vlcInstance = vlcInstance;
    m_vlcMediaPlayer = mediaPlayer;
}
