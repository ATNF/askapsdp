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
		
		/// @brief form vector of stokes enums from STOKES axis
        /// @return vector of stokes enums
        /// @note An axis names STOKES must be present, otherwise an exception will be thrown
        casa::Vector<casa::Stokes::StokesTypes> Axes::stokesAxis() const
        {
           ASKAPCHECK(has("STOKES"), "Stokes axis must be present in the axes object to be able to use stokesAxis");
           const int index = order("STOKES");
           const int stokesStart = int(start()[index]);
           const int stokesEnd = int(end()[index]);
           ASKAPCHECK((stokesStart>=0) && (stokesStart<1024),
                "Unable to interpret the start value="<< stokesStart <<" of the stokes axis");
           ASKAPCHECK((stokesEnd>=0) && (stokesEnd<1024),
                "Unable to interpret the end value="<< stokesEnd <<" of the stokes axis");
           
           // a bit C-like way of packing polarisation frame into two double numbers, but
           // it is probably better than having a separate code handling the stokes axis.
           // 31 == 0x1f means that this polarisation is undefined         
           std::vector<int> packedPol(4,31);
           packedPol[0] = stokesStart % 32;
           packedPol[1] = stokesStart / 32;
           packedPol[2] = stokesEnd % 32;
           packedPol[3] = stokesEnd / 32;
           size_t nPol = 0;
           for (size_t pol = 0; pol<packedPol.size(); ++pol) {
                if (packedPol[pol] == 31) {
                    break;
                }
                ++nPol;
           }           
           casa::Vector<casa::Stokes::StokesTypes> result(nPol,casa::Stokes::Undefined);
           for (size_t pol = 0; pol<result.nelements(); ++pol) {
                result[pol] = casa::Stokes::StokesTypes(packedPol[pol]);
           }
           return result;    
        }
		
		
		/// @brief add STOKES axis formed from the vector of stokes enums
        /// @details This is a reverse operation to extractStokesAxis. If the STOKES axis
        /// already exists, the values are updated.
        /// @param[in] stokes a vector of stokes enums
        void Axes::addStokesAxis(const casa::Vector<casa::Stokes::StokesTypes> &stokes)
        {
            ASKAPCHECK(stokes.nelements()<=4, "Only up to 4 polarisation products are supported");
            ASKAPCHECK(stokes.nelements()>0, "Unable to add stokes axis using an empty stokes vector");
            // a bit C-like way of packing polarisation frame into two double numbers, but
            // it is probably better than having a separate code handling the stokes axis.
            // 31 == 0x1f means that this polarisation is undefined
            std::vector<int> packedPol(4,31);
            for (size_t pol=0; pol<stokes.nelements(); ++pol) {
                 const int curDescriptor = int(stokes[pol]);
                 ASKAPCHECK((curDescriptor<0x1f) && (curDescriptor>0), "Stokes = "<<curDescriptor<<
                            " is not supported");
                 packedPol[pol] = curDescriptor;
            }
            const int start = 32*packedPol[1]+packedPol[0];
            const int end = 32*packedPol[3]+packedPol[2];
            
            if (has("STOKES")) {                
                update("STOKES", start, end);
            } else {
                add("STOKES", start, end);
            }
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
