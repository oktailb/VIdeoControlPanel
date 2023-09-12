#include "vlcmediaplayer.h"

VlcMediaPlayer::VlcMediaPlayer(QString networkCachingMs, bool hwDecoding, QObject *parent) :
    QMediaPlayer(parent),
    videoWidget(nullptr)
{
    int postProcLevel = 1;

    QString networkCaching = "--network-caching=" + networkCachingMs;
    const char *const vlc_args[] = {
        "--no-xlib",
        hwDecoding?"--avcodec-hw":"--no-hardware-decode",
        "--quiet",
        "--verbose=0",
        networkCaching.toStdString().c_str(),
        "--postproc-q", QString::number(postProcLevel).toUtf8().constData()
    };

    libVlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
    libVlcMediaPlayer = libvlc_media_player_new(libVlcInstance);
}

VlcMediaPlayer::~VlcMediaPlayer()
{
    libvlc_media_player_stop(libVlcMediaPlayer);
    libvlc_media_player_release(libVlcMediaPlayer);
    libvlc_release(libVlcInstance);
}

libvlc_media_player_t *VlcMediaPlayer::core()
{
    return this->libVlcMediaPlayer;
}

void VlcMediaPlayer::updateVideo() {
    if (videoWidget) {
        videoWidget->update();
    }
}

extern "C" {
    static void handleLibVlcEvent(const libvlc_event_t *event, void *data) {
        VlcMediaPlayer *player = static_cast<VlcMediaPlayer *>(data);
        switch (event->type) {
            case libvlc_MediaPlayerVout:
                QMetaObject::invokeMethod(player, "updateVideo", Qt::QueuedConnection);
                break;
        }
    }
}

void VlcMediaPlayer::setMedia(const QUrl &url)
{
    setSource(url);
}

void VlcMediaPlayer::setSource(const QUrl &url)
{
    libvlc_media_t *media = libvlc_media_new_location(libVlcInstance, url.toString().toUtf8().constData());
    libvlc_media_player_set_media(libVlcMediaPlayer, media);
    libvlc_media_release(media);
    libvlc_event_manager_t *eventManager = libvlc_media_player_event_manager(libVlcMediaPlayer);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerVout, handleLibVlcEvent, this);
}

void VlcMediaPlayer::setVideoOutput(QWidget *videoWidget)
{
    this->videoWidget = videoWidget;
}

void VlcMediaPlayer::play()
{
    if (videoWidget) {
#ifdef Q_OS_WIN
        libvlc_media_player_set_hwnd(libVlcMediaPlayer, reinterpret_cast<void *>(videoWidget->winId()));
#else
        libvlc_media_player_set_xwindow(libVlcMediaPlayer, static_cast<uint32_t>(videoWidget->winId()));
#endif
    }
    libvlc_media_player_play(libVlcMediaPlayer);
}

void VlcMediaPlayer::pause()
{
    libvlc_media_player_pause(libVlcMediaPlayer);
}

void VlcMediaPlayer::stop()
{
    libvlc_media_player_stop(libVlcMediaPlayer);
}

void VlcMediaPlayer::setPosition(qint64 position)
{
    libvlc_media_player_set_time(libVlcMediaPlayer, position);
}

void VlcMediaPlayer::setVolume(int volume)
{
    libvlc_audio_set_volume(libVlcMediaPlayer, volume);
}
