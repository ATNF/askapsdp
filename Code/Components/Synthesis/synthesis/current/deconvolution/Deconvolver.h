/// @file
/// @brief Interface class for a deconvolver
/// @details This interface class defines a deconvolver used to estimate an
/// image from a dirty image, psf optionally using a mask and a weights image.
/// @ingroup Deconvolver
///  
///
/// @copyright (c) 2007 CSIRO
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
/// @author Max Voronkov <maxim.voronkov@csiro.au>
///

#ifndef I_DECONVOLVER_H
#define I_DECONVOLVER_H
#include <casa/aips.h>
#include <boost/shared_ptr.hpp>

#include <casa/Arrays/Array.h>

#include <string>

#include <deconvolution/DeconvolverState.h>

namespace askap {

namespace synthesis {

/// @brief Interface class for a deconvolver
/// @details This interface class defines a deconvolver used to estimate an
/// image from a dirty image, psf optionally using a mask and a weights image.
/// The template argument T is the type, and FT is the transform
/// e.g. Deconvolver<Double, DComplex>
/// @ingroup Deconvolver
template<class T, class FT> class Deconvolver {

public:
    typedef boost::shared_ptr<Deconvolver<T> > ShPtr;

    virtual ~Deconvolver();

    /// @brief Construct from dirty image and psf
    /// @detailed Construct a deconvolver from a dirty image and
    /// the corresponding PSF
    /// @param[in] dirty Dirty image (array)
    /// @param[in] psf Point Spread Function (array)
    Deconvolver(const Array<T>& dirty, const Array<T>& psf);

    /// @brief Update only the dirty image
    /// @detailed Update an existing deconvolver for a changed dirty image
    /// @param[in] dirty Dirty image (array)
    void updateDirty(const Array<T>& dirty);

    void setMask(const Array<T> & mask) {
    	itsMask = mask;
    }

    void setWeight(const Array<T> & weight) {
    	itsWeight = mask;
    }

    const Array<T> & mask() const
    {
        return itsMask;
    }

    const Array<T> & weight() const
    {
        return itsWeight;
    }

    boost::shared_ptr<DeconvolverControl<T> > DC() const
    {
        return itsDC;
    }

    boost::shared_ptr<DeconvolverMonitor<T> > DM() const
    {
        return itsDM;
    }

    boost::shared_ptr<DeconvolverState<T> > DS() const
    {
        return itsDS;
    }

    void setDC(boost::shared_ptr<DeconvolverControl<T> > DC)
    {
        itsDC = DC;
    }

    void setDM(boost::shared_ptr<DeconvolverMonitor<T> > DM)
    {
        itsDM = DM;
    }

    void setDS(boost::shared_ptr<DeconvolverState<T> > DS)
    {
        itsDS = DS;
    }

private:

    /// The state of the deconvolver
    boost::shared_ptr<DeconvolverState<T> > itsDS;

    /// The control used for the deconvolver
    boost::shared_ptr<DeconvolverControl<T> > itsDC;

    /// The monitor used for the deconvolver
    boost::shared_ptr<DeconvolverMonitor<T> > itsDM;

    const Array<T>& itsDirty;

    const Array<T>& itsPSF;

    const Array<FT>& itsXFR;

    const Array<T>& itsMask;

    const Array<T>& itsWeight;

};

} // namespace synthesis

} // namespace askap

#endif  // #ifndef I_DECONVOLVER_H


