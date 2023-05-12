/*
  ==============================================================================

    Filter.h
    Created: 5 May 2023 4:27:40pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#pragma once

#include <juce_dsp/juce_dsp.h>

class Filter : public juce::dsp::LadderFilter<float>
{
  public:
    void updateCoefficients(float cutoff, float Q)
    {
        setCutoffFrequencyHz(cutoff);
        setResonance(std::clamp(Q / 30.0f, 0.0f, 1.0f));
    }

    float render(float x)
    {
        updateSmoothers();
        return processSample(x, 0);
    }
};

/*
 This is the initially implemented Cytomic SVF filter.
 */
// class Filter
//{
// public:
//     float sampleRate;
//
//     void updateCoefficients(float cutoff, float Q)
//     {
//         g = std::tan(PI * cutoff / sampleRate);
//         k = 1.0f / Q;
//         a1 = 1.0f / (1.0f + g * (g + k));
//         a2 = g * a1;
//         a3 = g * a2;
//     }
//
//     void reset()
//     {
//         g = 0.0f;
//         k = 0.0f;
//         a1 = 0.0f;
//         a2 = 0.0f;
//         a3 = 0.0f;
//
//         ic1eq = 0.0f;
//         ic2eq = 0.0f;
//     }
//
//     float render(float x)
//     {
//         float v3 = x - ic2eq;
//         float v1 = a1 * ic1eq + a2 * v3;
//         float v2 = ic2eq + a2 * ic1eq + a3 * v3;
//
//         ic1eq = 2.0f * v1 - ic1eq;
//         ic2eq = 2.0f * v2 - ic2eq;
//
//         return v2;
//     }
//
// private:
//     const float PI = 3.1415926535897932f;
//
//     float g, k, a1, a2, a3;
//     float ic1eq, ic2eq;
// };
