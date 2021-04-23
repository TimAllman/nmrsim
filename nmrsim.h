//
//  nmrsim.h
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
#ifndef NMRSIM_H
#define NMRSIM_H

#include "ProNmr.h"
#include "gendata.h"

#include <fstream>
#include <cstring>
#include <iosfwd>

bool getInputSpecs(const char *pFName, unsigned &iLines,
             float &fDwell, float &fDe, float *pAmplitude,
             float *pFreq, float *pDamp, float *pPhase);

int createData(const char *pInpFName, const char* pOutFNameRoot);

void writeGnuplot(const FloatArray& data, std::ostream& os);
void writeGnuplot(const ComplexfArray& data, std::ostream& os);

#endif // NMRSIM_H
