#include "sound.h"
#include "user.h"
#include "global.h"
#include <QAudioOutput>
#include <QMediaPlayer>


void Sound::playBleep()
{
    static QMediaPlayer *player = []() {
        auto player = new QMediaPlayer;
        auto audioOutput = new QAudioOutput;
        audioOutput->setVolume(80);
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl::fromLocalFile(maru::appResourcePath("assets/sounds/bleep.wav")));
        return player;
    }();

    if (User::load().soundEnable()) {
        player->stop();
        player->play();
    }
}


void Sound::playSnap()
{
    static QMediaPlayer *player = []() {
        auto player = new QMediaPlayer;
        auto audioOutput = new QAudioOutput;
        audioOutput->setVolume(80);
        player->setAudioOutput(audioOutput);
        player->setSource(QUrl::fromLocalFile(maru::appResourcePath("assets/sounds/snap.wav")));
        return player;
    }();

    if (User::load().soundEnable()) {
        player->stop();
        player->play();
    }
}
