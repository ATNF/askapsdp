/// @file
///
/// IVisWeights: Abstract base class for
///              Interface definition for visibility weight calculators
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Urvashi Rau <rurvashi@aoc.nrao.edu>
///
#ifndef ASKAP_SYNTHESIS_IVISWEIGHTS_H_
#define ASKAP_SYNTHESIS_IVISWEIGHTS_H_

#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>

#include <fitting/Axes.h>

#include <boost/shared_ptr.hpp>
#include <dataaccess/SharedIter.h>

#include <boost/shared_ptr.hpp>

namespace askap
{
	namespace synthesis
	{

		/// @brief Abstract Base Class for all visibility weight calculators.
		/// Visibilities can be weighted in various ways while implenenting
		/// matched filtering methods that search for the presence or absence
		/// of a particular pattern.
		/// 
		/// @ingroup gridding
		class IVisWeights
		{
			public:

			/// Shared pointer definition
			typedef boost::shared_ptr<IVisWeights> ShPtr;

			IVisWeights();
			virtual ~IVisWeights();
			
			/// Clone a copy of this Gridder
			virtual ShPtr clone() = 0;

			/// @brief Set the context
			/// @param order The index of the enumerated expansion
			virtual void setParameters(int order)=0;
			
			/// @brief Calculate the visibility weight.
			/// @param i Sample Index
			/// @param freq Frequency
			/// @param pol Polarization index
			virtual float getWeight(int i, double freq, int pol) = 0;

		};
	}
}
#endif                                            /*VISWEIGHTS_H_*/
