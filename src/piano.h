#ifndef PIANO_H_
#define PIANO_H_

#include "mesh.h"
#include "scene.h"
#include <cassert>

static const char *notes[] = {
        "assets/piano_sounds/A0.mp3",
        "assets/piano_sounds/B0.mp3",
        "assets/piano_sounds/C1.mp3",
        "assets/piano_sounds/D1.mp3",
        "assets/piano_sounds/E1.mp3",
        "assets/piano_sounds/F1.mp3",
        "assets/piano_sounds/G1.mp3",
        "assets/piano_sounds/A1.mp3",
        "assets/piano_sounds/B1.mp3",
        "assets/piano_sounds/C2.mp3",
        "assets/piano_sounds/D2.mp3",
        "assets/piano_sounds/E2.mp3",
        "assets/piano_sounds/F2.mp3",
        "assets/piano_sounds/G2.mp3",
        "assets/piano_sounds/A2.mp3",
        "assets/piano_sounds/B2.mp3",
        "assets/piano_sounds/C3.mp3",
        "assets/piano_sounds/D3.mp3",
        "assets/piano_sounds/E3.mp3",
        "assets/piano_sounds/F3.mp3",
        "assets/piano_sounds/G3.mp3",
        "assets/piano_sounds/A3.mp3",
        "assets/piano_sounds/B3.mp3",
        "assets/piano_sounds/C4.mp3",
        "assets/piano_sounds/D4.mp3",
        "assets/piano_sounds/E4.mp3",
        "assets/piano_sounds/F4.mp3",
        "assets/piano_sounds/G4.mp3",
        "assets/piano_sounds/A4.mp3",
        "assets/piano_sounds/B4.mp3",
        "assets/piano_sounds/C5.mp3",
        "assets/piano_sounds/D5.mp3",
        "assets/piano_sounds/E5.mp3",
        "assets/piano_sounds/F5.mp3",
        "assets/piano_sounds/G5.mp3",
        "assets/piano_sounds/A5.mp3",
        "assets/piano_sounds/B5.mp3",
        "assets/piano_sounds/C6.mp3",
        "assets/piano_sounds/D6.mp3",
        "assets/piano_sounds/E6.mp3",
        "assets/piano_sounds/F6.mp3",
        "assets/piano_sounds/G6.mp3",
        "assets/piano_sounds/A6.mp3",
        "assets/piano_sounds/B6.mp3",
        "assets/piano_sounds/C7.mp3",
        "assets/piano_sounds/D7.mp3",
        "assets/piano_sounds/E7.mp3",
        "assets/piano_sounds/F7.mp3",
        "assets/piano_sounds/G7.mp3",
        "assets/piano_sounds/A7.mp3",
        "assets/piano_sounds/B7.mp3",
        "assets/piano_sounds/C8.mp3",
        "assets/piano_sounds/Bb0.mp3",
        "assets/piano_sounds/Db1.mp3",
        "assets/piano_sounds/Eb1.mp3",
        "assets/piano_sounds/Gb1.mp3",
        "assets/piano_sounds/Ab1.mp3",
        "assets/piano_sounds/Bb1.mp3",
        "assets/piano_sounds/Db2.mp3",
        "assets/piano_sounds/Eb2.mp3",
        "assets/piano_sounds/Gb2.mp3",
        "assets/piano_sounds/Ab2.mp3",
        "assets/piano_sounds/Bb2.mp3",
        "assets/piano_sounds/Db3.mp3",
        "assets/piano_sounds/Eb3.mp3",
        "assets/piano_sounds/Gb3.mp3",
        "assets/piano_sounds/Ab3.mp3",
        "assets/piano_sounds/Bb3.mp3",
        "assets/piano_sounds/Db4.mp3",
        "assets/piano_sounds/Eb4.mp3",
        "assets/piano_sounds/Gb4.mp3",
        "assets/piano_sounds/Ab4.mp3",
        "assets/piano_sounds/Bb4.mp3",
        "assets/piano_sounds/Db5.mp3",
        "assets/piano_sounds/Eb5.mp3",
        "assets/piano_sounds/Gb5.mp3",
        "assets/piano_sounds/Ab5.mp3",
        "assets/piano_sounds/Bb5.mp3",
        "assets/piano_sounds/Db6.mp3",
        "assets/piano_sounds/Eb6.mp3",
        "assets/piano_sounds/Gb6.mp3",
        "assets/piano_sounds/Ab6.mp3",
        "assets/piano_sounds/Bb6.mp3",
        "assets/piano_sounds/Db7.mp3",
        "assets/piano_sounds/Eb7.mp3",
        "assets/piano_sounds/Gb7.mp3",
        "assets/piano_sounds/Ab7.mp3",
        "assets/piano_sounds/Bb7.mp3",
};


class PianoKey : public Mesh
{
    private:
        int note_id;

    public:
        PianoKey(int id, int a, bool b, bool c, bool d, float e)
        : Mesh("PianoKey", a, b, c, d, e),
          note_id(id)
        {
                assert(static_cast<size_t>(note_id) < sizeof notes / sizeof notes[0]);
        }

        void play()
        {
                ((Scene *) scene)->repr_audio(notes[note_id]);
        }
};

#endif // PIANO_H_
