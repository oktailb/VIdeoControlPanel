#ifndef VLCMEDIAPLAYER_H
#define VLCMEDIAPLAYER_H

#include <QObject>
#include <QWidget>
#include <QUrl>
#include <vlc/vlc.h>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimediaWidgets/QtMultimediaWidgets>

class VlcMediaPlayer : public QMediaPlayer
{
    Q_OBJECT
public slots:
    void updateVideo(); // Ajouter cette ligne
public:
    explicit VlcMediaPlayer(QString networkCachingMs, bool hwDecoding, QObject *parent = nullptr);
    ~VlcMediaPlayer();

    libvlc_media_player_t *core();
    void setMedia(const QUrl &url);
    void setSource(const QUrl &url);
    void setVideoOutput(QWidget *videoWidget);
    void play();
    void pause();
    void stop();
    void setPosition(qint64 position);
    void setVolume(int volume);

signals:
    void stateChanged();
    void mediaStatusChanged();
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);

private:
    QWidget *videoWidget;
    libvlc_instance_t *libVlcInstance;
    libvlc_media_player_t *libVlcMediaPlayer;
};

#endif // VLCMEDIAPLAYER_H
