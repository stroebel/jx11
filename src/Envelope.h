/*
  ==============================================================================

    Envelope.h
    Created: 10 Apr 2023 8:41:33pm
    Author:  Jaco Stroebel

  ==============================================================================
*/

#pragma once

const float SILENCE = 0.0001f; // Globals are bad for audio programming

class Envelope
{
  public:
    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;

    float level;

    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }

    float nextValue()
    {
        level = multiplier * (level - target) + target;

        if (level + target > 3.0f)
        {
            multiplier = decayMultiplier;
            target = sustainLevel;
        }

        return level;
    }

    void release()
    {
        target = 0.0f;

        multiplier = releaseMultiplier;
    }

    void attack()
    {
        level += SILENCE + SILENCE;
        target = 2.0f;
        multiplier = attackMultiplier;
    }

    inline bool isActive() const { return level > SILENCE; }

    inline bool isInAttack() const { return target >= 2.0f; }

  private:
    float target;
    float multiplier;
};
