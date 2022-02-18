#include "sound.h"
#include <QSoundEffect>


void Sound::playBleep()
{
    QSoundEffect effect;

    //if (!effect.isLoaded()) {
        effect.setSource(QUrl::fromLocalFile("assets/sounds/bleep.wav"));
        effect.setLoopCount(1);
        effect.setVolume(1.0);
    //}
    effect.play();
}


void Sound::playSnap()
{
    QSoundEffect effect;

    //if (!effect.isLoaded()) {
        effect.setSource(QUrl::fromLocalFile("assets/sounds/snap.wav"));
        effect.setLoopCount(1);
        effect.setVolume(1.0);
    //};
    effect.play();
}
