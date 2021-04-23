//
//  DataGenerator.cpp
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
#include "DataGenerator.h"
#include "DataGenerator.h"
#include "ProNmr.h"

#include <fstream>
#include <stdexcept>

DataGenerator::DataGenerator(const InputSpecs& specs)
    : mSpecs(specs)
{
}

DataGenerator::InputSpecs::InputSpecs(DataGenerator::OutputFormat format, const std::string& fName)
    : mFormat(format), mFName(fName)
{

}

DataGenerator::InputSpecs::InputSpecs(const DataGenerator::InputSpecs& specs)
    : mSpecs(specs)
{
}

void DataGenerator::InputSpecs::read()
{
    try
    {
        std::ifstream is(mFName, std::ios::);
        is.exceptions(std::ifstream::failbit);
    }
    catch (std::ios_base::failure& fail)
    {
        std::cerr << "Unable to open file: " << mFName
                  << "\n" << fail.what() << std::endl;
        throw;
    }

    try
    {
        // get the single parameters: dwell then de
        is >> mFidSize >> mDwell >> mPreDelay;

        mNLines = 0;
        while (is)
        {
            float amplitude, freq, damp,  phase;
            // check return value to deal with whitespace at end of file
            is << amplitude << freq << damp << phase;
            mAmplitude.push_back(amplitude);
            mFreq.push_back(freq);
            mDamp.push_back(damp);
            mPhase.push_back(phase);
            mNLines++;
        }
    }

    catch (std::ios_base::failure& fail)
    {
        std::cerr << "Failure reading file: " << mFName
                  << "\n" << fail.what() << std::endl;
        throw;
    }
}

void DataGenerator::InputSpecs::init()
{
    mFormat = NONE;
    mFName = "";
    mFidSize = 0;
    mNLines = 0;
    mDwell = 0.0;
    mPreDelay = 0.0;
    mAmplitude.clear();
    mFreq.clear();
    mDamp.clear();
    mPhase.clear();
}

std::string DataGenerator::InputSpecs::fName() const
{
    return mFName;
}

int DataGenerator::InputSpecs::fidSize() const
{
    return mFidSize;
}

int DataGenerator::InputSpecs::nLines() const
{
    return mNLines;
}

float DataGenerator::InputSpecs::dwell() const
{
    return mDwell;
}

float DataGenerator::InputSpecs::preDelay() const
{
    return mPreDelay;
}

float DataGenerator::InputSpecs::amplitude() const
{
    return mAmplitude;
}

float DataGenerator::InputSpecs::freq() const
{
    return mFreq;
}

float DataGenerator::InputSpecs::damp() const
{
    return mDamp
}

float DataGenerator::InputSpecs::phase() const
{
    return mPhase;
}


DataGenerator::DataGenerator(const DataGenerator::InputSpecs& specs)
{

}

void DataGenerator::generate()
{
    if (mSpecs.format() == PRONMR)
    {
        generateProNmrFid();
        return;
    }

    if (mSpecs.format() == RANGER)
    {
        generateRanger();
        return;
    }

    throw std::invalid_argument("Invalid format specification." << "");
}

void DataGenerator::generateProNmrFid()
{
    ProNmr fileInfo;

    mSpecs.read();

    std::cout << "Read " << mSpecs.nLines() << " peaks." << std::endl;

    // make some complex fids
    ComplexfArray ComplexFid(mSpecs.fidSize());

    // Generate FID
    makeSimFid(ComplexFid, mSpecs.nLines(), mSpecs.dwell(), mSpecs.amplitude(),
               mSpecs.freq(), mSpecs.damp(), mSpecs.phase(), mSpecs.dwell(), true);

        // Add the required noise
        addNoise(ComplexFid, pNoise[iSpec]);

        // write out as a gnuplot data set
        sprintf(pOutFName, "data/%s-%3.2f.gp", pOutFNameRoot, pNoise[iSpec]);
        std::ofstream os(pOutFName);
        if (!os)
        {
            printf("Could not open file: %s\n", pOutFName);
            return false;
        }

        writeGnuplot(ComplexFid, os);
        os.close();

        // stream out so that it can be read later.
        sprintf(pOutFName, "data/%s-%3.2f", pOutFNameRoot, pNoise[iSpec]);
        os.open(pOutFName);
        //ComplexFid.write(os);
        os << ComplexFid;
        os.close();

        // now write out as a pronmr file
        strcat(pOutFName, "p");
        if (Header.writeParams(pOutFName) != 0)
        {
            exit(1);
        }

        float *pData = reinterpret_cast<float *>(ComplexFid.data());
        int iNData = ComplexFid.rows() * 2;
        if (Header.writeData(pData, pOutFName, iNData, Header.offsets[DAT], 1, 1)!= 0)
        {
            exit(1);
        }
    }

    return 0;

}

void DataGenerator::generateRanger()
{

}

void DataGenerator::addExpDecaySim(ComplexfArray& fid, float dwell, float amplitude, float freq,
                                   float damp, float phase, float de, bool zeroarray)
{
    // convert input parameters to radians
    phase *= M_PI / 180;
    freq *= M_PI;

    float dw_decay = exp(damp * dwell);           /* decay per dwell */
    float dw_angle = dwell * fFreq;                /* evolution per dwell */
    float decay = exp(damp * de);                 /* starting decay */
    float angle = freq * de + phase;              /* starting angle */

    if (zeroarray)
        fid.fill(0.0);

    /* off we go */
    for (unsigned i = 0; i < fid.size(); i++)
    {
        fid(i) += amplitude * cos(angle) * decay;
        fid(i) += amplitude * sin(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
    }
}

void DataGenerator::addExpDecaySeq(FloatArray& fid, float dwell, float amplitude, float freq, float damp, float phase, float de, bool zeroarray)
{
    // convert input parameters to radians
    phase *= M_PI / 180;
    freq *= 2.0 * M_PI;

    float dw_decay = exp(damp * dwell);           /* decay per dwell */
    float dw_angle = dwell * fFreq;                /* evolution per dwell */
    float decay = exp(damp * de);                 /* starting decay */
    float angle = freq * de + phase;              /* starting angle */

    if (zeroarray)
        fid.fill(0.0);

    /* off we go */
    for (unsigned i = 0; i < fid.size(); i += 2)
    {
        fid(i) += amplitude * cos(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
        fid(i+1) -= amplitude * sin(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
    }
}

void DataGenerator::addExpDecaySin(FloatArray& fid, float dwell, float amplitude, float freq, float damp, float phase, float de, bool zeroarray)
{
    // convert input parameters to radians
    phase *= M_PI / 180;
    freq *= 2.0 * M_PI;

    float dw_decay = exp(damp * dwell);           /* decay per dwell */
    float dw_angle = dwell * fFreq;                /* evolution per dwell */
    float decay = exp(damp * de);                 /* starting decay */
    float angle = freq * de + phase;              /* starting angle */

    if (zeroarray)
        fid.fill(0.0);

    /* off we go */
    for (unsigned i = 0; i < fid.size(); i += 2)
    {
        fid(i) += amplitude * cos(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
        fid(i+1) -= amplitude * sin(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
    }
}

void DataGenerator::makeSimFid(ComplexfArray& fid, unsigned nlines, float dwell, float* amplitude,
                               float* freq, float* damp, float* phase, float phase_0, float de, bool zerofid)
{

}

void DataGenerator::makeSeqFid(FloatArray& fid, unsigned nlines, float dwell, float* amplitude, float* freq, float* damp, float* phase, float phase_0, float de, bool zerofid)
{

}

void DataGenerator::makeSinFid(FloatArray& fid, unsigned nlines, float dwell, float* amplitude, float* freq, float* damp, float* phase, float phase_0, float de, bool zerofid)
{

}

float DataGenerator::uniformDeviate()
{

}

float DataGenerator::gaussianDeviate(float mean, float variance)
{

}

void DataGenerator::addNoise(FloatArray& fid, float noiseLevel)
{

}

void DataGenerator::addNoise(ComplexfArray& Fid, float noiseLevel)
{

}

/**********----------**********----------**********/
void DataGenerator::addExpDecaySim(ComplexfArray &fid, float dwell, float amplitude,
                       float freq, float damp, float phase, float de,
                       bool zeroarray)
{
    // convert input parameters to radians
    float fPhase = phase * 2.0 * M_PI / 360.0;
    float fFreq = freq * 2.0 * M_PI;

    float dw_decay = exp(damp * dwell);           /* decay per dwell */
    float dw_angle = dwell * fFreq;                /* evolution per dwell */
    float decay = exp(damp * de);                 /* starting decay */
    float angle = fFreq * de + fPhase;              /* starting angle */

    if (zeroarray)
        fid.fill(0.0);

    /* off we go */
    for (unsigned i = 0; i < fid.size(); i++)
    {
        fid(i) += amplitude * cos(angle) * decay;
        fid(i) += amplitude * sin(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
    }
}


/**********----------**********----------**********/
void DataGenerator::addExpDecaySeq(FloatArray &fid, float dwell, float amplitude,
         float freq, float damp, float phase, float de, bool zeroarray)
{
    // convert input parameters to radians
    phase *= 2.0 * M_PI;
    freq *= 2.0 * M_PI;

    float dw_decay = exp(damp * dwell);           /* decay per dwell */
    float dw_angle = dwell * freq;                /* evolution per dwell */
    float decay = exp(damp * de);                 /* starting decay */
    float angle = freq * de + phase;              /* starting angle */

    if (zeroarray)
        fid.fill(0.0);

    /* off we go */
    for (unsigned i = 0; i < fid.size(); i += 2)
    {
        fid(i) += amplitude * cos(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
        fid(i+1) -= amplitude * sin(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
    }
}

/**********----------**********----------**********/
void DataGenerator::addExpDecaySin(FloatArray &fid, float dwell, float amplitude,
         float freq, float damp, float phase, float de, bool zeroarray)
{
    // convert input parameters to radians
    phase *= 2.0 * M_PI;
    freq *= 2.0 * M_PI;

    float dw_decay = exp(damp * dwell);           /* decay per dwell */
    float dw_angle = dwell * freq;                /* evolution per dwell */
    float decay = exp(damp * de);                 /* starting decay */
    float angle = freq * de + phase;              /* starting angle */

    if (zeroarray)
        fid.fill(0.0);

    /* off we go */
    for (unsigned i = 0; i < fid.size(); i++)
    {
        fid(i) += amplitude * cos(angle) * decay;
        decay *= dw_decay;
        angle += dw_angle;
    }
}

/**********----------**********----------**********/
void DataGenerator::makeSimFid(ComplexfArray &fid, unsigned nlines, float dwell,
               float *amplitude, float *freq, float *damp,
               float *phase, float phase_0, float de, bool zerofid)

/* Use the above adddecay() to generate a complete fid.  The
   parameters are in the arrays which must have been initialised
   with at least nlines entries.  If zeroarray is not FALSE then
   the array will be cleared before proceeding */

{
    if (zerofid)
        fid.fill(Complexf(0.0, 0.0));

    for (unsigned i = 0; i < nlines; i++)
        addExpDecaySim(fid, dwell, amplitude[i], freq[i],
                       damp[i], phase[i] + phase_0, de, false);
}

/**********----------**********----------**********/
void DataGenerator::makeSeqFid(FloatArray &fid, unsigned nlines, float dwell,
               float *amplitude, float *freq, float *damp,
               float *phase, float phase_0, float de, bool zerofid)

/* Use the above adddecay() to generate a complete fid.  The
   parameters are in the arrays which must have been initialised
   with at least nlines entries.  If zeroarray is not false then
   the array will be cleared before proceeding */

{
    unsigned i;

    if (zerofid)
        fid.fill(0.0);

    for (unsigned i = 0; i < nlines; i++)
        AddExpDecaySeq(fid, dwell, amplitude[i], freq[i],
                          damp[i], phase[i] + phase_0, de, false);
}

/**********----------**********----------**********/
void DataGenerator::makeSinFid(FloatArray &fid, unsigned nlines, float dwell,
                 float *amplitude, float *freq, float *damp,
                 float *phase, float de, bool zerofid)

/* Use the above adddecay() to generate a complete fid.  The
   parameters are in the arrays which must have been initialised
   with at least nlines entries.  If zeroarray is not false then
   the array will be cleared before proceeding */

{
    if (zerofid)
        fid.fill(0.0);

    for (unsigned i = 0; i < nlines; i++)
        AddExpDecaySin(fid, dwell, amplitude[i], freq[i],
                          damp[i], phase[i] + phase_0, de, false);
}

// generate a uniform deviate in the range [0, 1]
float DataGenerator::uniformDeviate()
{
    // random(1) generates a long in the range [0, 2**31-1] so we have to scale
    // it back a bit
    const float MAX_RAND = 0xffffffff / 2;
    return float(random()) / MAX_RAND;
}

// generate a unit gaussian using the central limit theorem
float DataGenerator::gaussianDeviate(float mean, float variance)
{
    // the more the better
    const unsigned NLOOPS = 20;

    float fNum = 0.0;
    for (unsigned iIdx = 0; iIdx < NLOOPS; iIdx++)
    {
        fNum += float(uniformDeviate());
    }
    //for uniform randoms in [0,1], mu = 0.5 and var = 1/12
    // adjust X so mu = 0 and var = 1
    fNum -= NLOOPS / 2;          // set mean to 0.0
    fNum *= sqrt(12.0 / NLOOPS); // adjust variance to 1.0

    // set mean and variance to requested values
    return (mean + sqrt(variance) * fNum);
}

void DataGenerator::addNoise(FloatArray &Fid, float fStdDev)
{
    for (unsigned iIdx = 0; iIdx < Fid.rows(); iIdx++)
    {
        Float fNum = gaussianDeviate(0.0, fStdDev * fStdDev);
        Fid(iIdx) += fNum;
    }
}

void DataGenerator::addNoise(ComplexfArray &Fid, float fStdDev)
{
    for (unsigned iIdx = 0; iIdx < Fid.rows(); iIdx++)
    {
        Float fNum1 = gaussianDeviate(0.0, fStdDev * fStdDev);
        Float fNum2 = gaussianDeviate(0.0, fStdDev * fStdDev);
        Fid(iIdx) += Complexf(fNum1, fNum2);
    }
}
