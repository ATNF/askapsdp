/// @file
///
/// VisGridderFactory: Factory class for visibility
//  gridders.
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell <tim.cornwell@csiro.au>
///
#ifndef VISGRIDDERFACTORY_H_
#define VISGRIDDERFACTORY_H_

#include <gridding/IVisGridder.h>

#include <APS/ParameterSet.h>

namespace askap
{
	namespace synthesis
	{
		/// @brief Factory class for visibility gridders
		/// @ingroup gridding
		class VisGridderFactory
		{
			public:
				/// @brief Factory Class for all gridders.
				/// @todo Python version of factory 
				VisGridderFactory();
				virtual ~VisGridderFactory();

				/// @brief Make a shared pointer for a visibility gridder
				/// @param parset ParameterSet containing description of
				/// gridder to be constructed
				static IVisGridder::ShPtr make(
				    const LOFAR::ACC::APS::ParameterSet& parset);
		};

	}
}
#endif 
