#include "audioplay.h"
#include <QDebug>
Audioplay::Audioplay(QWidget *parent) :
    QWidget(parent)
{

   // relay_handle = ::open("/sys/class/leds/eint16/brightness",O_WRONLY);
   // timeout.setInterval(30000);
    connect(&playtimer,SIGNAL(timeout()),this,SLOT(stop_music()));
}

void Audioplay::play_music(int addr, int flag)
{
    if(audioisplay == 1)
       stop_music();

    audioisplay = 1;
    playtimer.start(30000);

    pid = fork();
    if(pid < 0)
    {
        audioisplay = 0;
        playtimer.stop();
        //return;
    }
    else if(pid == 0)
    {
        open_relay();
        char pmusic_code[50];

        if(flag == 0)
        {
            sprintf(pmusic_code,"/waterpump/audio/%derror.wav",addr);
            execl("/usr/bin/madplay","madplay","-r","/waterpump/audio/alarm.mp3",pmusic_code,(char *)0);
        }
        else
        {
            sprintf(pmusic_code,"/waterpump/audio/%doffwater.wav",addr);
            execl("/usr/bin/madplay","madplay","-r","/waterpump/audio/alarm.mp3",pmusic_code,(char *)0);
        }
    }
    else
        return;
}


void Audioplay::play_welcome()
{
    playtimer.start(10000);
    audioisplay = 1;
    pid = fork();
    if(pid < 0)
    {
        qDebug()<< "fork error";
        audioisplay = 0;
        playtimer.stop();
    }
    else if(pid == 0)
    {
        qDebug()<< "in the child process1";
        open_relay();
        qDebug()<< "in the child process2";
        execl("/usr/bin/madplay","madplay","/waterpump/audio/welcome.mp3",(char *)0);
    }
    else
    {
        qDebug()<< "in the main process";
        return;
    }
}

void Audioplay::audio_init()
{
    relay_handle = ::open("/sys/class/leds/eint16/brightness",O_WRONLY);
    audioisplay = 0;
}

void Audioplay::stop_music()
{
    audioisplay = 0;
    playtimer.stop();
    close_relay();
    system("killall -9 madplay");
    int status;
    waitpid(-1,&status,0);
    if (0 == WEXITSTATUS(status))
    {
        printf("run successfully.\n");
    }
    else
    {
        printf("exit code: %d\n", WEXITSTATUS(status));
    }
}
void Audioplay::open_relay()
{
    write(relay_handle,"1",1);
}
void Audioplay::close_relay()
{
    write(relay_handle,"0",1);
}
