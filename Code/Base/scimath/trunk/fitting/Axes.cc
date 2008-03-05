/// @file
///
/// Axes: Provides Axes definitions for fitting parameters
///
/// (c) 2007 ASKAP, All Rights Reserved.
/// @author Tim Cornwell tim.cornwel@csiro.au
///
#include <fitting/Axes.h>
#include <casa/aips.h>
#include <casa/Exceptions/Error.h>

#include <askap/AskapError.h>

#include <vector>
#include <string>
#include <iostream>

#include <Blob/BlobOStream.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobArray.h>
#include <Blob/BlobSTL.h>

using std::ostream;
using std::string;
using std::vector;

namespace askap
{
	namespace scimath
	{

		/// Make an empty axes
		Axes::Axes()
		{
		}

		/// Assignment operator
		Axes& Axes::operator=(const Axes& other)
		{
			if(this!=&other)
			{
				itsNames=other.itsNames;
				itsStart=other.itsStart;
				itsEnd=other.itsEnd;
			}
			return *this;
		}

		/// Copy constructor
		Axes::Axes(const Axes& other)
		{
			operator=(other);
		}

		Axes::~Axes()
		{
		}

		void Axes::add(const std::string& name, const double start, const double end)
		{
			ASKAPCHECK(!has(name), "Axis " + name + " already exists");
			itsNames.push_back(name);
			itsStart.push_back(start);
			itsEnd.push_back(end);
		}

		bool Axes::has(const std::string& name) const
		{
			if(itsNames.size()==0) return false;
			for (uint i=0;i<itsNames.size();i++)
			{
				if(itsNames[i]==name) return true;
			}
			return false;
		}

		int Axes::order(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Axis " + name + " does not exist");
			for (uint i=0;i<itsNames.size();i++)
			{
				if(itsNames[i]==name) return i;
			}
                        return -1;
		}

		const std::vector<string>& Axes::names() const
		{
			return itsNames;
		}

		double Axes::start(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Axis " + name + " does not exist");
			return itsStart[order(name)];
		}

		double Axes::end(const std::string& name) const
		{
			ASKAPCHECK(has(name), "Axis " + name + " does not exist");
			return itsEnd[order(name)];
		}

		// Return start values
		const std::vector<double>& Axes::start() const
		{
			return itsStart;
		}

		// Return end values
		const std::vector<double>& Axes::end() const
		{
			return itsEnd;
		}

		std::ostream& operator<<(std::ostream& os, const Axes& axes)
		{

			vector<string> names(axes.names());
			for(vector<string>::const_iterator it = names.begin();
					it != names.end(); it++)
			{
				os << *it << " from " << axes.start(*it) << " to " << axes.end(*it)
				<< std::endl;
			}
			return os;
		}

		LOFAR::BlobOStream& operator<<(LOFAR::BlobOStream& os, const Axes& axes)
		{
			os << axes.itsNames << axes.itsStart << axes.itsEnd;
                        return os;
		}

		LOFAR::BlobIStream& operator>>(LOFAR::BlobIStream& is, Axes& axes)
		{
			is >> axes.itsNames >> axes.itsStart >> axes.itsEnd;
                        return is;
		}

	}

};
