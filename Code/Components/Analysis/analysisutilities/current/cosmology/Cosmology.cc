/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2011 CSIRO
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
/// @author XXX XXX <XXX.XXX@csiro.au>
///
#include <askap_analysisutilities.h>

#include <cosmology/Cosmology.h>
#include <math.h>

namespace askap {

namespace analysisutilities {

namespace cosmology {

Cosmology::Cosmology():
    itsHubble(HUBBLE_WMAP), itsOmegaM(OMEGAM_WMAP), itsOmegaL(OMEGAL_WMAP)
{
}

Cosmology::Cosmology(const double hubble, const double omegaM, const double omegaL):
    itsHubble(hubble), itsOmegaM(omegaM), itsOmegaL(omegaL)
{
}


const double Cosmology::dlum(const double z)
{
    double dz = z / double(NUMINT);
    double rr = 0.;
    for (int i = 0; i < NUMINT; i++) {
        double zp1 = (i + 0.5) * dz + 1;
        double temp = itsOmegaL +
                      ((1. - itsOmegaL - itsOmegaM) * (zp1 * zp1)) +
                      (itsOmegaM * (zp1 * zp1 * zp1));
        double drdz = 1. / sqrt(temp);
        rr = rr + drdz * dz;
    }

    double dl = rr * (1. + z) * (C_ms / 1000.) / itsHubble; /* dlum in Mpc */
    dl = dl * MPC_m;                                        /* dlum in metres */

    return log10(dl);

}

const double Cosmology::lum(const double z, const double flux)
{
    return log10(4.*M_PI) + 2.*this->dlum(z) + flux;
}

}

}

}


