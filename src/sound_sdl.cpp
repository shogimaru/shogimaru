#include "sound.h"
#include <QString>
#include <QDebug>
#include <SDL/SDL_mixer.h>
#include <mutex>

constexpr int UsedChannels = 2;  // 使用オーディオチャンネル数


class Audio {
public:
    Audio(const QString &path, int channel = 0);
    ~Audio();

    void play();
    void stop();

private:
    Mix_Chunk *_chunk {nullptr};
    int _channel {-1};
};


Audio::Audio(const QString &path, int channel) :
    _channel(channel)
{
    static std::once_flag once;
    std::call_once(once, []() {
        Mix_Init(0);
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, UsedChannels, 4096) < 0) {
            qCritical() << "Failed Mix_OpenAudio()";
        }
    });

    _chunk = Mix_LoadWAV(qPrintable(path));
    if (_chunk) {
        _chunk->volume = 128;
    }
}


Audio::~Audio()
{
    Mix_FreeChunk(_chunk);
}


void Audio::play()
{
    if (_chunk) {
        Mix_PlayChannelTimed(_channel, _chunk, 0, -1);
    }
}


void Audio::stop()
{
    if (_chunk) {
        Mix_HaltChannel(_channel);
    }
}


void Sound::playBleep()
{
    static Audio bleep("assets/sounds/bleep.wav", 0);
    bleep.play();
}


void Sound::playSnap()
{
    static Audio bleep("assets/sounds/snap.wav", 1);
    bleep.play();
}
