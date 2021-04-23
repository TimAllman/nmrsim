//
//  Nmr86.h
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
#ifndef PRONMR_H
#define PRONMR_H

//#include <QtCore/qglobal.h>
#include <string>

#define PBLOCKSIZE 256         /* Size of aq params in bytes */
#define MAXSIZE 32768          /* Largest SI allowed */
#define PPBLOCK 32

enum blocktype
{
    ACQU, PROC, TWOD,
    UNUSED1, UNUSED2, UNUSED3, UNUSED4,
    DAT
};

/* Define the bits in the data status word */
#define FT_DONE    1       /* bit 0, set if ft done */
#define WIN_DONE   2       /* bit 1, set if window done */
#define BC_DONE    4       /* bit 2, set if bc done */
#define AQ_SEQ     8       /* bit 3, set if sequential acquisition */
#define AQ_SIM    16       /* bit 4, set if simultaneous acq.
                              both 3 & 4 clear if single channel */
#define SHUFF     32       /* bit 5, set if shuffled */
#define FT2_DONE  64       /* bit 6, set if FT2 performed */
#define FT1_DONE 128       /* bit 7, set if FT1 performed */
#define HYPER_COMPLEX 256  /* bit 8, set if hypercomplex MTX */

/* Define the first block of acq. parameters */

class ProNmr
{
public:
    ProNmr();

    void init();

    /* Writes the file ACQU parameters to the disk and checks to make sure
       that all went well.  Returns 0 if all went well. */
    void writeParams(const std::string& name);

    /*
      name:      Full file name to write to
      size:      Number of data to read (need not be a power of 2)
      datoffset: Offset (units of SECSIZE bytes) of start of data
      blocknum:  Block number to get in serial files (1 based)
      nspec:     Number of spectra to read at once

      Allows writing serial files as well as ordinary ones.
      Returns 0 if all went well.

      The array is passed as a **float because dynamic memory
      reallocation is done internally and the address of the
      data array may change.

      This is a somewhat abbreviated routine which reads in the
      acquisition parameters and data only.  It does not have
      the full functionality of the corresponding Prospector
      function.
    */
    int writeData(float *data, char *name, int size, int datoffset, int blocknum,
                int nspec);

    void createData(const std::string& inputFName, const std::string& outputFNameRoot);

    /* Key word to check for validity */
    char keyname[8];   /* MUST == "\005NMR86" */
    char fileversion;       /* File format version number. */
    char offsets[8];        /* Offsets, in blocks of SECSIZE bytes,
                                      of the various sets of parameters. */
    unsigned short si;     /* Size of dataset */
    unsigned short td;     /* Number of points collected */
    unsigned short nrecs;  /* Number of components in file */
    unsigned short dstatus;/* Current state of file */
    unsigned short ns;      /* Number of scans */
    double
        sf,                /* Spectrometer frequency */
        o1,                /* Obs. offset */
        dw;                /* Dwell period, (s) */
    float
        de;                /* Preacquisition delay */
    char
        filename[30],      /* Name of file when stored */
        machineid[30],     /* Name of acq. machine */
        date[31],          /* Some text comments */
        comment1[41],
        comment2[41];
    float vd;          /* Variable delay */
    double in2d;       /* Minimal 2D param. set */
    double sf1;
    double o11;
};

#endif // PRONMR_H
