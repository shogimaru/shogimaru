#include "sound.h"
#include <QMediaPlayer>
#include <QAudioOutput>


void Sound::playBleep()
{
    static QMediaPlayer *player = []() {
        auto player = new QMediaPlayer;
        auto audioOutput = new QAudioOutput;
        audioOutput->setVolume(80);
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl::fromLocalFile("assets/sounds/bleep.wav"));
        return player;
    }();
    player->stop();
    player->play();
}


void Sound::playSnap()
{
    static QMediaPlayer *player = []() {
        auto player = new QMediaPlayer;
        auto audioOutput = new QAudioOutput;
        audioOutput->setVolume(80);
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl::fromLocalFile("assets/sounds/snap.wav"));
        return player;
    }();
    player->stop();
    player->play();
}
