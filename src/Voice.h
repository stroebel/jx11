/*
  ==============================================================================

    Voice.h
    Created: 27 Mar 2023 8:29:25pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#pragma once

#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

struct Voice
{
    int note;
    int lastNote;
    float saw;
    float period;
    float panLeft, panRight;
    float target;
    float glideRate;
    float cutoff;
    float filterMod;
    float filterQ;
    float pitchBend;
    float filterEnvDepth;

    Oscillator osc1;
    Oscillator osc2;
    Filter filter;

    Envelope env;
    Envelope filterEnv;

    void reset()
    {
        note = 0;
        saw = 0.0f;
        panLeft = 0.707f;
        panRight = 0.707f;
        osc1.reset();
        osc2.reset();
        env.reset();
        filter.reset();
        filterEnv.reset();
    }

    float render(float input)
    {
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();
        saw = saw * 0.997f + sample1 - sample2;

        float output = saw + input;
        output = filter.render(output);

        float envelope = env.nextValue();

        return output * envelope;
    }

    void release()
    {
        env.release();
        filterEnv.release();
    }

    void updatePanning()
    {
        float panning = std::clamp((note - 60.0f) / 24.0f, -1.0f, 1.0f);
        panLeft = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
    }

    void updateLFO()
    {
        period += glideRate * (target - period);

        float fenv = filterEnv.nextValue();

        float modulatedCutoff = cutoff * std::exp(filterMod + filterEnvDepth + fenv) / pitchBend;

        modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
        filter.updateCoefficients(modulatedCutoff, filterQ);
    }
};
