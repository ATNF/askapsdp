/// @file Visibilities.cc
///
/// @copyright (c) 2009 CSIRO
/// Australia Telescope National Facility (ATNF)
/// Commonwealth Scientific and Industrial Research Organisation (CSIRO)
/// PO Box 76, Epping NSW 1710, Australia
/// atnf-enquiries@csiro.au
///
/// This file is part of the ASKAP software distribution.
///
/// The ASKAP software distribution is free software: you can redistribute it
/// and/or modify it under the terms of the GNU General Public License as
/// published by the Free Software Foundation; either version 2 of the License,
/// or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program; if not, write to the Free Software
/// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
///
/// @author Ben Humphreys <ben.humphreys@csiro.au>

#ifndef ASKAP_CP_VIS_H
#define ASKAP_CP_VIS_H

const unsigned int n_baselines = 666;
const unsigned int n_beams = 32;
const unsigned int n_coarse_chan = 19;

const unsigned int n_fine_per_coarse = 54;
const unsigned int n_pol = 4;

struct FloatComplex
{
    float real;
    float imag;
};

struct Visibilities
{
    /// Timestamp - Binary Atomic Time (BAT). The number of microseconds
    /// since Modified Julian Day (MJD) = 0
    unsigned long timestamp;

    /// Coarse Channel - Which coarse channel this block of data relates to.
    unsigned int coarseChannel;

    /// First antenna
    unsigned int antenna1;

    /// Second antenna
    unsigned int antenna2;

    /// First beam
    unsigned int beam1;

    /// Second beam
    unsigned int beam2;

    /// Visibilities
    FloatComplex vis[n_fine_per_coarse * n_pol];

    /// The number of voltage samples that made up the visibility for this
    /// integration. This has the same dimension as "vis. i.e. one nSamples
    /// value per visibility in the "vis" array. An nSamples value of zero for
    /// any channel/polarization indicates that visibility has been flagged by
    /// the correlator as bad.
    unsigned int nSamples[n_fine_per_coarse * n_pol];
};

#endif
