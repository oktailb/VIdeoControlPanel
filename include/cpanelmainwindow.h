#ifndef CPANELMAINWINDOW_H
#define CPANELMAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QDebug>
#include <QGridLayout>
//#include <QtMultimedia/QMultimedia>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtMultimediaWidgets/QtMultimediaWidgets>
#include "vlcmediaplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class CPanelMainWindow; }
QT_END_NAMESPACE

class CPanelMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    CPanelMainWindow(QWidget *parent = nullptr);
    ~CPanelMainWindow();
//    void capture();
public slots:
    void onConfigChanged(const QString &path);
//    void stream();

private:
    Ui::CPanelMainWindow *ui;
    QMap<QString, QString>              configuration;
    QFileSystemWatcher *watcher;
    QElapsedTimer m_timer;
    QTimer m_resyncTimer;
    int resyncThreshold;
    int resyncInterval;
    const QString conf = "configuration.ini";
    void readConfiguration();
    void stop();
    void startRtspStream();
    QImage captureWindow();
    static int openMediaCb(void *opaque, void **datap, uint64_t *sizep);
    static ssize_t readMediaCb(void *opaque, unsigned char *buf, size_t len);
    static int seekMediaCb(void *opaque, uint64_t offset);
    static void closeMediaCb(void *opaque);
    libvlc_instance_t *m_vlcInstance;
    libvlc_media_player_t *m_vlcMediaPlayer;
    QMap<QString, QMediaPlayer *> players;
    QMap<QString, QGraphicsProxyWidget *> proxies;
    QVector<QThread *> threads;
    QImage pix;

    using MessageHandler = QString (CPanelMainWindow::*)(const QString&);
    QMap<QString, MessageHandler> handlers;

    QString updateVideoUrl(const QString &message);
    QString onRecord(const QString &message);
    QString onPause(const QString &message);
    QString onUnpause(const QString &message);
    QString onStop(const QString &message);
    void resyncPlayers();
    void playSynchronized(QMediaPlayer *player, int delay);

protected:
    void keyPressEvent(QKeyEvent *event);

};
#endif // CPANELMAINWINDOW_H
