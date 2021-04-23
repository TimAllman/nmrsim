/***************************************************************************
*                                                                          *
*         Copyright (c) 1993 SoftPulse Software. All rights reserved.      *
*                                                                          *
*         This is unpublished source code that remains the property of     *
*         SoftPulse Software. It may not be sold, given away, published    *
*         or in any way transfered without written permission from         *
*         SoftPulse Software.                                              *
*                                                                          *
****************************************************************************/

/* This module contains the routines necessary to generate model FIDs
   and spectra. */


#include "gendata.h"
#include "nmrsim.h"

#include <fstream>

using namespace std;

bool getInputSpecs(const char *pFName, unsigned &iLines,
             float &fDwell, float &fDe, float *pAmplitude,
             float *pFreq, float *pDamp, float *pPhase)
{
    FILE *pFile = fopen(pFName, "r");

    if (pFile == 0)
    {
        printf("Could not open file: %s\n", pFName);
        return false;
    }

    // get the single parameters: dwell then de
    fscanf(pFile, "%f %f", &fDwell, &fDe);

    iLines = 0;
    while (!feof(pFile))
    {
        // check return value to deal with whitespace at end of file
        if (fscanf(pFile, "%f %f %f %f",
               &pAmplitude[iLines], &pFreq[iLines], &pDamp[iLines],
               &pPhase[iLines]) == 4)
            iLines++;
    }

    return true;
}

int createData(const char *pInpFName, const char* pOutFNameRoot)
{
    std::ofstream os;
    char pOutFName[200];

    // containers for the data in the file
    const int MAXLINES = 50;
    const unsigned NDWELLS = 1024;
    const float PHASE_0 = 0.0;

    unsigned iLines;
    float fDwell;
    float fDe;
    float pAmplitude[MAXLINES];
    float pFreq[MAXLINES];
    float pDamp[MAXLINES];
    float pPhase[MAXLINES];

    for (int i = 0; i < MAXLINES; i++)
    {
        pAmplitude[i] = -999.0;
        pFreq[i] = -999.0;
        pDamp[i] = -999.0;
        pPhase[i] = -999.0;
    }

    const unsigned NSPECS = 10;

    float pNoise[NSPECS] = {
        0.00, 0.01, 0.02, 0.04, 0.08, 0.16, 0.32, 0.64, 1.28, 2.56
    };

    ProNmr Header;
    memset(&Header, 0, sizeof(ProNmr));

    // now fill what we can here
    strcpy(Header.keyname, "\005NMR86");
    Header.fileversion = 2;
    Header.offsets[ACQU] = 0;
    Header.offsets[DAT] = 2;
    Header.si = NDWELLS * 2;
    Header.td = NDWELLS * 2;
    Header.nrecs = 1;
    Header.dstatus |= AQ_SIM | SHUFF;
    Header.ns = 1;
    Header.sf = 100.0e6;
    Header.o1 = 0.0;
    Header.dw = 0.001;
    Header.de = 0.0;
    strcpy(Header.filename, "\008filename");
    strcpy(Header.machineid, "\008Sim data");
    time_t Now = time(0);
    strcpy(Header.date, ctime(&Now));
    Header.vd = 0.0;
    Header.in2d = 0.0;
    Header.sf1 = 0.0;
    Header.o11= 0.0;

    if (getInputSpecs(pInpFName, iLines, fDwell, fDe, pAmplitude,
                pFreq, pDamp, pPhase) == false)
        exit(1);

    printf("Read %d peaks.\n", iLines);

    // make some complex fids
    ComplexfArray ComplexFid(NDWELLS);
    for (unsigned iSpec = 0; iSpec < NSPECS; iSpec++)
    {
        // Generate FID
        MakeSimFid(ComplexFid, iLines, fDwell, pAmplitude, pFreq, pDamp,
                    pPhase, PHASE_0, fDe, true);

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

void writeGnuplot(const FloatArray& array, std::ostream& os)
{
    // We put out the array as a matrix, ignoring dim2 for the time being.
    for (unsigned iDim0 = 0; iDim0 < array.rows(); iDim0++)
    {
        for (unsigned iDim1 = 0; iDim1 < array.cols(); iDim1++)
        {
            // across the rows
            if (iDim1 != 0)
                os << ' ';
            os << array(iDim0, iDim1);
        }
        os << "\n";
    }

    os << std::endl;
}

void writeGnuplot(const ComplexfArray& array, std::ostream& os)
{
    // We put out the array as a matrix, ignoring dim2 for the time being.
    for (unsigned iDim0 = 0; iDim0 < array.rows(); iDim0++)
    {
        for (unsigned iDim1 = 0; iDim1 < array.cols(); iDim1++)
        {
            // across the rows
            if (iDim1 != 0)
                os << ' ';
            os << array(iDim0, iDim1).real();
        }
        os << "\n";
    }

    os << std::endl;
}
