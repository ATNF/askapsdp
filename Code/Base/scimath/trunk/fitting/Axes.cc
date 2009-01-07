/// @file
///
/// Axes: Provides Axes definitions for fitting parameters
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
/// @author Tim Cornwell <tim.cornwell@csiro.au>
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
		
		/// @brief update an axis
        /// @details Sometimes it is handy to modify one axis only without resorting to 
        /// axis by axis copy. This method assigns new start and end values to a given
        /// axis. It is equivalent to add if the required axis doesn't exist.
        /// @param[in] name name of axis
        /// @param[in] start start value 
        /// @param[in] end end value
        void Axes::update(const std::string &name, const double start, const double end)
        {
            int axisNumber = -1; // flag showing that required axis does not exist
            for (size_t i=0;i<itsNames.size();++i) {
                 if (itsNames[i] == name) {
                     axisNumber = int(i);
                     break;
                 }
            }
            if (axisNumber == -1) {
                add(name,start,end);
            } else {
                itsStart[axisNumber] = start;
                itsEnd[axisNumber] = end;
            }
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
