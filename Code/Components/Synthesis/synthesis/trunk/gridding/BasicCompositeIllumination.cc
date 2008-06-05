/// @file 
/// @brief Basic composite illumination pattern
/// @details This class is implements an basic composite illumination pattern corresponding
/// to a given weights and offsets of physical feeds. It can be used for simulation and/or
/// imaging with a synthetic beam. As an implementation of IBasicIllumination interface, 
/// this class provides a method to obtain illumination pattern by populating a pre-defined 
/// grid supplied as a UVPattern object. It looks like handling of illumination patterns
/// inside gridders has to be generalised (i.e. main method should receive a full accessor
/// with all the metadata instead of just the pointing offsets, frequency, etc). Such 
/// transition would definitely require an interface change in this class.
///
/// @copyright (c) 2008 ASKAP, All Rights Reserved.
/// @author Max Voronkov <maxim.voronkov@csiro.au>

#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/ArrayMath.h>


#include <gridding/BasicCompositeIllumination.h>

#include <askap/AskapError.h>

namespace askap {

namespace synthesis {

/// @brief construct the pattern using given weights and offsets
/// @param[in] pattern single-feed illumination pattern (assumed the same for all feeds)
/// @param[in] feedOffsets offsets of physical feeds in radians
/// @param[in] weights complex weights for each feed
/// @note The size of two vectors should be the same
BasicCompositeIllumination::BasicCompositeIllumination(const boost::shared_ptr<IBasicIllumination> &pattern,
            const casa::Vector<casa::RigidVector<casa::Double, 2> > &feedOffsets,
            const casa::Vector<casa::Complex> &weights) : itsPattern(pattern),
            itsFeedOffsets(feedOffsets), itsWeights(weights)
{
   ASKAPDEBUGASSERT(itsPattern);
   ASKAPDEBUGASSERT(itsFeedOffsets.nelements() == itsWeights.nelements());
}

/// @brief obtain illumination pattern
/// @details This is the main method which populates the 
/// supplied uv-pattern with the values corresponding to the model
/// represented by this object. It has to be overridden in the 
/// derived classes. An optional phase slope can be applied to
/// simulate offset pointing.
/// @param[in] freq frequency in Hz for which an illumination pattern is required
/// @param[in] pattern a UVPattern object to fill
/// @param[in] l angular offset in the u-direction (in radians)
/// @param[in] m angular offset in the v-direction (in radians)
void BasicCompositeIllumination::getPattern(double freq, UVPattern &pattern, double l, 
                           double m) const
{
   itsPattern->getPattern(freq,pattern,l,m);
   // now apply the phase screen appropriate to the feed configuration/weights
   
   const casa::uInt oversample = pattern.overSample();
   const double cellU = pattern.uCellSize()/oversample;
   const double cellV = pattern.vCellSize()/oversample;
       
   // sizes of the grid to apply the phase screen to
   const casa::uInt nU = pattern.uSize();
   const casa::uInt nV = pattern.vSize();
   // number of feeds
   const casa::uInt nFeeds = itsWeights.nelements();
   
   double sum=0.; // normalisation factor
   
        
   for (casa::uInt iU=0; iU<nU; ++iU) {
	    const double offsetU = double(iU)-double(nU)/2.;
		for (casa::uInt iV=0; iV<nV; ++iV) {
	         const double offsetV = double(iV)-double(nV)/2.;
	         casa::Complex weight(0.,0.);
             for (casa::uInt iFeed = 0; iFeed<nFeeds; ++iFeed) {
                  // don't need to multiply by wavelength here because the
			      // illumination pattern is given
			      // in a relative coordinates in frequency
                  const double phase = 2.*casa::C::pi*(cellU *itsFeedOffsets[iFeed](0)*offsetU+
                                    cellV *itsFeedOffsets[iFeed](1)*offsetV);
                  weight+=itsWeights[iFeed]*casa::Complex(cos(phase), -sin(phase));
			 }
			 pattern(iU, iV) *= weight;
			 sum += std::abs(weight);
		}
	}
	
	
    ASKAPCHECK(sum > 0., "Integral of the synthetic pattern should be non-zero");
    pattern.pattern() *= casa::Complex(float(nU)*float(nV)/float(sum),0.); 
}

} // namespace synthesis

} // namespace askap
