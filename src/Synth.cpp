/*
  ==============================================================================

    Synth.cpp
    Created: 27 Mar 2023 8:30:02pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#include "Synth.h"
#include "Utils.h"

static const float ANALOG = 0.002f;
static const int SUSTAIN = -1;

Synth::Synth()
{
    sampleRate = 44100.0f; // sample rate in Hz
}

void Synth::allocateResources(double sampleRate_, int samplesPerBlock)
{
    sampleRate = static_cast<float>(sampleRate_);
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].filter.setMode(juce::dsp::LadderFilterMode::LPF12);
        voices[v].filter.prepare(spec);
    }
}

void Synth::deallocateResources()
{
    // do nothing
}

void Synth::reset()
{
    pitchBend = 1.0f;
    sustainPedalPressed = false;
    outputLevelSmoother.reset(sampleRate, 0.05);
    lfo = 0.0f;
    lfoStep = 0;
    modWheel = 0.0f;
    lastNote = 0;
    pressure = 0.0f;
    filterCtl = 0.0f;
    filterZip = 0.0f;

    for (int v = 0; v < MAX_VOICES; ++v)
        voices[v].reset();

    noiseGen.reset();
}

void Synth::render(float **outputBuffers, int sampleCount)
{
    float *outputBufferLeft = outputBuffers[0];
    float *outputBufferRight = outputBuffers[1];

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice &voice = voices[v];
        if (voice.env.isActive())
        {
            voice.osc1.period = voice.period * pitchBend;
            voice.osc2.period = voice.osc1.period * detune;
            voice.glideRate = glideRate;
            voice.filterQ = filterQ;
            voice.pitchBend = pitchBend;
            voice.filterEnvDepth = filterEnvDepth;
        }
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        const float noise = noiseGen.nextValue() * noiseMix;
        updateLFO();
        float outputLeft = 0.0f;
        float outputRight = 0.0f;

        for (int v = 0; v < MAX_VOICES; ++v)
        {
            Voice &voice = voices[v];

            if (voice.env.isActive())
            {
                float output = voice.render(noise);

                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
        }

        float outputLevel = outputLevelSmoother.getNextValue();

        outputLeft *= outputLevel;
        outputRight *= outputLevel;

        if (outputBufferRight != nullptr)
        {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }
        else
        {
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
        }
    }
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice &voice = voices[v];

        if (!voice.env.isActive())
        {
            voice.env.reset();
            voice.filter.reset();
        }
    }

    protectYourEars(outputBufferLeft, sampleCount);
    protectYourEars(outputBufferRight, sampleCount);
}

void Synth::midiMesage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    {
    case 0x80: // Note off
        noteOff(data1 & 0x7F);
        break;
    case 0x90:
    { // Note on
        uint8_t note = data1 & 0x7F;
        uint8_t velo = data2 & 0x7F;

        if (velo > 0)
        {
            noteOn(note, velo);
        }
        else
        {
            noteOff(note);
        }
        break;
    }
    case 0xE0:
        pitchBend = std::exp(-0.000014102f * float(data1 + 128 * data2 - 8192));
        break;
    case 0xB0:
        controlChange(data1, data2);
        break;
    case 0x01:
        modWheel = 0.000005f * float(data2 * data2);
        break;
    case 0xD0:
        pressure = 0.0001f * float(data1 * data2);
        break;
    }
}

void Synth::startVoice(int v, int note, int velocity)
{
    Voice &voice = voices[v];
    Envelope &env = voice.env;
    Envelope &filterEnv = voice.filterEnv;

    int noteDistance = 0;

    float period = calcPeriod(v, note);
    float vel = 0.004f * float(velocity + 64) * (velocity + 64) - 8.0f;

    voice.note = note;
    voice.updatePanning();
    voice.target = period;
    voice.osc1.amplitude = vel * volumeTrim;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    voice.cutoff = sampleRate / (period * PI);
    voice.cutoff *= std::exp(velocitySensitivity * float(velocity - 64));

    if (lastNote > 0)
    {
        if ((glideMode == 2) || ((glideMode == 1) && isPlayingLegatoStyle()))
        {
            noteDistance = note - lastNote;
        }
    }
    voice.period = period * std::pow(1.059463094359f, float(noteDistance) - glideBend);

    if (voice.period < 6.0f)
        voice.period = 6.0f;

    lastNote = note;
    voice.note = note;
    voice.updatePanning();

    if (vibrato == 0.0f && pwmDepth > 0.0f)
    {
        voice.osc2.squareWave(voice.osc1, voice.period);
    }

    filterEnv.attackMultiplier = filterAttack;
    filterEnv.decayMultiplier = filterDecay;
    filterEnv.sustainLevel = filterSustain;
    filterEnv.releaseMultiplier = filterRelease;
    filterEnv.attack();

    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::noteOn(int note, int velocity)
{
    int v = 0;

    if (ignoreVelocity)
        velocity = 80;

    if (numVoices == 1)
    {
        if (voices[0].note > 0)
        {
            shiftQueuedNotes();
            restartMonoVoice(note, velocity);
            return;
        }
    }
    else
    {
        v = findFreeVoice();
    }

    startVoice(v, note, velocity);
}

void Synth::noteOff(int note)
{
    if ((numVoices == 1) && (voices[0].note == note))
    {
        int queuedNote = nextQueuedNote();
        if (queuedNote > 0)
        {
            restartMonoVoice(queuedNote, -1);
        }
    }

    for (int v = 0; v < MAX_VOICES; v++)
    {
        if (voices[v].note == note)
        {
            if (sustainPedalPressed)
            {
                voices[v].note = SUSTAIN;
            }
            else
            {
                voices[v].release();
                voices[v].note = 0;
            }
        }
    }
}

float Synth::calcPeriod(int v, int note) const
{
    float period = tune * std::exp2(-0.05776226505f * (float(note) + ANALOG * float(v)));

    while (period < 6.0f || (period * detune) < 6.0f)
    {
        period += period;
    }

    return period;
}

int Synth::findFreeVoice() const
{
    int v = 0;
    float l = 100.0f;

    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (voices[i].env.level < l && !voices[i].env.isInAttack())
        {
            l = voices[i].env.level;
            v = i;
        }
    }
    return v;
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    switch (data1)
    {
    case 0x40:
        sustainPedalPressed = (data2 >= 64);
        break;
    case 0x4A:
        filterCtl = 0.02f * float(data2);
        break;
    case 0x4B:
        filterCtl = -0.03f * float(data2);
        break;
    default:
        if (data1 >= 0x78)
        {
            for (int v = 0; v < MAX_VOICES; ++v)
            {
                voices[v].reset();
            }
            sustainPedalPressed = false;
        }
        break;
    }
}

void Synth::restartMonoVoice(int note, int velocity)
{
    float period = calcPeriod(0, note);

    Voice &voice = voices[0];
    voice.target = period;

    if (glideMode == 0)
        voice.period = period;

    voice.cutoff = sampleRate / (period * PI);

    if (velocity > 0)
        voice.cutoff *= std::exp(velocitySensitivity * float(velocity - 64));

    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.updatePanning();
}

void Synth::shiftQueuedNotes()
{
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--)
    {
        voices[tmp].note = voices[tmp - 1].note;
        voices[tmp].release();
    }
}

int Synth::nextQueuedNote()
{
    int held = 0;

    for (int v = MAX_VOICES - 1; v > 0; v--)
    {
        if (voices[v].note > 0)
        {
            held = v;
        }
    }

    if (held > 0)
    {
        int note = voices[held].note;

        voices[held].note = 0;

        return note;
    }

    return 0;
}

void Synth::updateLFO()
{
    if (--lfoStep <= 0)
    {
        lfoStep = LFO_MAX;

        lfo += lfoInc;

        if (lfo > PI)
            lfo -= TAU;

        const float sine = std::sin(lfo);

        float vibratoMod = 1.0f + sine * (modWheel + vibrato);
        float pwm = 1.0f + sine * (modWheel + pwmDepth);

        float filterMod = filterKeytracking + filterCtl + (filterLFODepth + pressure) * sine;

        filterZip += 0.005f * (filterMod - filterZip);

        for (int v = 0; v < MAX_VOICES; ++v)
        {
            Voice &voice = voices[v];

            if (voice.env.isActive())
            {
                voice.osc1.modulation = vibratoMod;
                voice.osc2.modulation = pwm;
                voice.filterMod = filterZip;
                voice.updateLFO();
                updatePeriod(voice);
            }
        }
    }
}

bool Synth::isPlayingLegatoStyle() const
{
    int held = 0;
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (voices[i].note > 0)
            held += 1;
    }

    return held > 0;
}
