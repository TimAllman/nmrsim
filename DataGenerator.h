//
//  DataGenerator.h
//  Ranger
//

/* Ranger is an NMR processing program.
 * Copyright © 2021 Tim Allman
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DATAGENERATOR_H
#define DATAGENERATOR_H

#include <Eigen/Dense>

#include <string>
#include <complex>
#include <vector>

using Float = float;
using FloatArray = Eigen::Matrix<Float, Eigen::Dynamic, 1>;

using Complexf = std::complex<Float>;
using ComplexfArray = Eigen::Matrix<Complexf, Eigen::Dynamic, 1>;

class DataGenerator
{
public:
    enum OutputFormat
    {
        NONE, PRONMR, RANGER
    };

    enum ReturnValue
    {
        SUCCESS, FAILURE
    };

    class InputSpecs
    {
    public:
        InputSpecs(OutputFormat format, const std::string& fName);
        InputSpecs(const InputSpecs& specs);
        void read();
        void init();
        OutputFormat format() const;
        std::string fName() const;
        int fidSize() const;
        int nLines() const;
        float dwell() const;
        float preDelay() const;
        float amplitude() const;
        float freq() const;
        float damp() const;
        float phase() const;

    private:
        OutputFormat mFormat;
        std::string mFName;
        int mFidSize;
        int mNLines;
        float mDwell;
        float mPreDelay;
        std::vector<float> mAmplitude;
        std::vector<float> mFreq;
        std::vector<float> mDamp;
        std::vector<float> mPhase;
    };

    DataGenerator(const InputSpecs& specs);

    void generate();
    void generateProNmrFid();
    void generateRanger();


/**
        Adds a decay to the data in fid.  The array is zeroed first if zeroarray != 0.

        fid          -- complex array of at least npts in length
        npts         -- number of time points to compute for FID
        dwell        -- dwell period (s)
        amplitude    -- amplitude (peak areas) of line (== value at time == 0)
        frequency    -- frequency (rotating frame) for each component (rad)
        damp         -- damping factor (1 / s)
        phase        -- phase of line at time == 0 (radians)
        de           -- pre-acq delay

*/
    void addExpDecaySim(ComplexfArray& fid, float dwell, float amplitude, float freq,
                        float damp, float phase, float de, bool zeroarray);

/**      Adds a sequential decay to the data in fid.  The array is zeroed
        first if zeroarray != 0.  We negate the "imaginary" channel
        to keep this consistent with Bruker conventions.

        fid          -- real array of at least npts in length
        dwell        -- dwell period (s)
        amplitude    -- amplitude (peak areas) of line (== value at time == 0)
        frequency    -- frequency (rotating frame) for each component (rad)
        damp         -- damping factor (1 / s)
        phase        -- phase of line at time == 0 (radians)
        de           -- pre-acq delay
*/

    void addExpDecaySeq(FloatArray& fid, float dwell, float amplitude, float freq,
                        float damp, float phase, float de, bool zeroarray);
/**      Adds a real decay to the data in fid.  The array is zeroed first if
        zeroarray != 0.

        fid          -- real array of at least fid.size() in length
        dwell        -- dwell period (s)
        amplitude    -- amplitude (peak areas) of line (== value at time == 0)
        frequency    -- frequency (rotating frame) for each component (rad)
        damp         -- damping factor (1 / s)
        phase        -- phase of line at time == 0 (radians)
        de           -- pre-acq delay
*/
    void addExpDecaySin(FloatArray& fid, float dwell, float amplitude, float freq,
                        float damp, float phase, float de, bool zeroarray);

/** Use the above addExpDecaySim() to generate a complete fid.  The
   parameters are in the arrays which must have been initialised
   with at least nlines entries.  If zeroarray is "true" then
   the array will be cleared before proceeding */
    void makeSimFid(ComplexfArray& fid, unsigned nlines, float dwell, float *amplitude,
                    float *freq, float *damp, float *phase, float de, bool zerofid);

/* Use the above AddExpDecaySeq() to generate a complete fid.  The
   parameters are in the arrays which must have been initialised
   with at least nlines entries.  If zeroarray is "true" then
   the array will be cleared before proceeding */
    void makeSeqFid(FloatArray& fid, unsigned nlines, float dwell, float *amplitude, float *freq,
                    float *damp, float *phase, float de, bool zerofid);

/* Use the above AddExpDecaySin() to generate a complete fid.  The
   parameters are in the arrays which must have been initialised
   with at least nlines entries.  If zeroarray is "true" then
   the array will be cleared before proceeding */
    void makeSinFid(FloatArray& fid, unsigned nlines, float dwell, float *amplitude, float *freq,
                    float *damp, float *phase, float de, bool zerofid);

// generate a uniform deviate in the range [0, 1]
    float uniformDeviate();

// generate a gaussian deviate with mean and variance
    float gaussianDeviate(float mean, float variance);

// add noise with standard deviation fNoiseLevel.
    void addNoise(FloatArray& fid, float noiseLevel);

// add noise with standard deviation fNoiseLevel.
    void addNoise(ComplexfArray& Fid, float noiseLevel);

private:
    InputSpecs mSpecs;
};

#endif // DATAGENERATOR_H
