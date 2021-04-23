//
//  Nmr86.cpp
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
#include "ProNmr.h"
#include "DataGenerator.h"

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>

ProNmr::ProNmr()
{
    init();
}

void ProNmr::init()
{
    // now fill what we can here
    strcpy(keyname, "\005NMR86");

    fileversion = 2;
    offsets[ACQU] = 0;
    offsets[DAT] = 2;
    si = NDWELLS * 2;
    td = NDWELLS * 2;
    nrecs = 1;
    dstatus |= AQ_SIM | SHUFF;
    ns = 1;
    sf = 100.0e6;
    o1 = 0.0;
    dw = 0.001;
    de = 0.0;
    strcpy(filename, "\008filename");
    strcpy(machineid, "\008Sim data");
    time_t Now = time(0);
    strcpy(date, ctime(&Now));
    vd = 0.0;
    in2d = 0.0;
    sf1 = 0.0;
    o11= 0.0;
}

void ProNmr::writeParams(const std::string& name)
{
    try
    {
        std::ofstream os(name, std::ios::binary);
        os.exceptions(std::ofstream::failbit);
    }
    catch (std::ios_base::failure& fail)
    {
        std::cerr << "Unable to open file: " << name
                  << "\n" << fail.what() << std::endl;
        throw;
    }

    /* Put the new AQ parameters */
    try
    {
        os.write(*this, sizeof(ProNmr));
    }

    catch (std::ios_base::failure &fail)
    {
        std::cerr << "Unable to write to file: " << name
                  << "\n" << fail.what() << std::endl;
        throw;
    }
}

int ProNmr::writeData(float* data, char* name, int size, int datoffset, int blocknum, int nspec)
{

#define POINTSPERSEC 32 /* Data points per sector */
#define SECSIZE 128     /* Bytes per hypothetical sector */

    unsigned pointstowrite, pointswritten, index,
        npoints, filepos, fileincr;
    FILE *f1 = fopen(name, "a");

    if (f1 == NULL) /* Open file for binary reading */
    {
        printf("Unable to write to file: %s\n", name);
        return(1);
    }

    /* At this point the file will be thought of as containing blocks
       of BLOCKSIZE points following the header.  In a MTX file
       the blocks for a given row of points are separated by fileincr
       blocks of points while in single files and .SER files they are
       contiguous.  Blocks are always aligned on these 32 point
       boundaries. */
    if (size < POINTSPERSEC)
        size = POINTSPERSEC;

    /* Number of Singles to read at once */
    fileincr = POINTSPERSEC;
    /* Position of first block */
    filepos = datoffset * SECSIZE / sizeof(float) + (blocknum - 1) * size;
    pointstowrite = size * nspec; /** Current total number left to read */

    /* The main writing loop */
    index = 0;
    while (pointstowrite > 0)
    {
        if (pointstowrite > POINTSPERSEC)
            npoints = POINTSPERSEC;
        else
            npoints = pointstowrite;
        fseek(f1, filepos * sizeof(float), SEEK_SET);
        pointswritten = fwrite(&data[index], sizeof(float), npoints, f1);
        if (pointswritten != npoints)
        {
            printf("Unable to write to file: %s\n", name);
            fclose(f1);
            return(1);
        }
        filepos += fileincr;
        index += pointswritten;
        pointstowrite -= pointswritten;
    }
    fclose(f1);
    return(0);
}

void ProNmr::createData(const std::string& inputFName, const std::string& outputFNameRoot)
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

