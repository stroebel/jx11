/*
  ==============================================================================

    Synth.h
    Created: 27 Mar 2023 8:30:02pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "Voice.h"
#include "NoiseGenerator.h"

class Synth
{
  public:
    Synth();

    bool ignoreVelocity;

    float noiseMix;
    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;
    float detune;
    float oscMix;
    float tune;
    float volumeTrim;
    float velocitySensitivity;
    float lfoInc;
    float vibrato;
    float pwmDepth;
    float glideRate;
    float glideBend;
    float filterKeytracking;
    float filterQ;
    float filterLFODepth;
    float filterAttack, filterDecay, filterSustain, filterRelease;
    float filterEnvDepth;

    juce::LinearSmoothedValue<float> outputLevelSmoother;

    int numVoices;
    int glideMode;

    static constexpr int MAX_VOICES = 8;
    const int LFO_MAX = 32;

    float calcPeriod(int v, int note) const;
    void allocateResources(double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();
    void render(float **outputBuffers, int sampleCount);
    void midiMesage(uint8_t data0, uint8_t data1, uint8_t data2);

  private:
    bool sustainPedalPressed;

    int lastNote;
    int lfoStep;

    float sampleRate;
    float pitchBend;
    float lfo;
    float modWheel;
    float pressure;
    float filterCtl;
    float filterZip;

    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGen;

    bool isPlayingLegatoStyle() const;

    int findFreeVoice() const;
    int nextQueuedNote();

    void startVoice(int v, int note, int velocity);
    void restartMonoVoice(int note, int velocity);
    void controlChange(uint8_t data1, uint8_t data2);
    void noteOn(int note, int velocity);
    void noteOff(int note);
    void shiftQueuedNotes();
    void updateLFO();

    inline void updatePeriod(Voice &voice)
    {
        voice.osc1.period = voice.period * pitchBend;
        voice.osc2.period = voice.osc1.period * detune;
    }
};
