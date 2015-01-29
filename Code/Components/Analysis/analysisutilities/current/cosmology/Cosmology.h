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
#ifndef ASKAP_ANALYSISUTIL_COSMOLOGY_H_
#define ASKAP_ANALYSISUTIL_COSMOLOGY_H_

namespace askap {

namespace analysisutilities {

namespace cosmology {

const float HUBBLE_WMAP = 71.;
const float OMEGAM_WMAP = 0.27;
const float OMEGAL_WMAP = 0.73;
const float MPC_m = 3.086e22;
const float C_ms = 299792458.;
const float C_kms = 299792.458;

const int NUMINT = 10000;

class Cosmology {
    public:
        Cosmology();
        Cosmology(const double hubble, const double omegaM, const double omegaL);
        virtual ~Cosmology() {};

        const double dlum(const double z);

        const double lum(const double z, const double flux);

    private:
        double itsHubble;
        double itsOmegaM;
        double itsOmegaL;

};

}

}

}



#endif //COSMOLOGY_H
