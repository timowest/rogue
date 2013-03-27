/*
 * rogue - multimode synth
 *
 * Copyright (C) 2013 Timo Westkämper
 *
 * This header is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; either version 2.1 of the License,
 * or (at your option) any later version.
 *
 * This header is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA.
 */

/**
 * ADSR envelope class.
 *
 * This class implements a traditional ADSR (Attack, Decay, Sustain,
 * Release) envelope. It responds to simple keyOn and keyOff
 * messages, keeping track of its state. The \e state = ADSR::IDLE
 * before being triggered and after the envelope value reaches 0.0 in
 * the ADSR::RELEASE state. All rate, target and level settings must
 * be non-negative. All time settings must be positive.
 */
class ADSR {

    enum { A, D, S, R, IDLE };

  public:
    void on() {
        state_ = A;
        last = 0.0;
    }

    void off() {
        state_ = R;
        releaseRate = last / releaseSamples;
    }

    void setADSR(float _a, float _d, float _s, float _r) {
        attackRate = attackTarget / _a;
        decayRate = (attackTarget - _s) / _d;
        sustain = _s;
        releaseSamples = _r;
    }

    double lastOut() { return last; }

    int state() { return state_; }

    double tick() {
        if (state_ == A) { // attack
          last += attackRate;
          if (last >= attackTarget) {
            last = attackTarget;
            state_ = D;
          }
        } else if (state_ == D) { // decay
          last -= decayRate;
          if (last <= sustain) {
            last = sustain;
            state_ = S;
          }
        } else if (state_ == R) { // release
          last -= releaseRate;
          if (last < 0.0) {
            last = 0.0;
            state_ = IDLE;
          }
        }
        return last;
    }

  private:
    double attackTarget = 1.0; // TODO : make modifiable
    double attackRate, decayRate, releaseSamples, releaseRate = 0.0;
    double sustain = 0.5;
    double last = 0.0;
    int state_ = IDLE;
};
