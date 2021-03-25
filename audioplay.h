#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H

#include <QWidget>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <QTimer>
class Audioplay : public QWidget
{
Q_OBJECT
public:
    explicit Audioplay(QWidget *parent = 0);
    void play_music(int addr, int flag);

    void play_welcome();
    void audio_init();
    void open_relay();
    void close_relay();

public slots:
    void stop_music();
private:
    int relay_handle;//继电器的句柄
    pid_t pid;
    int audioisplay;

    QTimer playtimer;
};

#endif // AUDIOPLAY_H
