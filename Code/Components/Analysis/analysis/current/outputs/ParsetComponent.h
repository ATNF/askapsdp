/// @file
///
/// Class to manage the data for a component written to a parset
///
/// @copyright (c) 2014 CSIRO
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
/// @author Matthew Whiting <Matthew.Whiting@csiro.au>
///
#ifndef ASKAP_ANALYSIS_PARSET_COMPONENT_H_
#define ASKAP_ANALYSIS_PARSET_COMPONENT_H_

#include <string>
#include <ostream>
#include <sourcefitting/RadioSource.h>


namespace askap {

    namespace analysis {

	class ParsetComponent
	{
	public:
	    ParsetComponent();
	    ParsetComponent(const ParsetComponent& other);
	    ParsetComponent& operator= (const ParsetComponent& other);
	    virtual ~ParsetComponent(){};

	    void setHeader(duchamp::FitsHeader *head){itsHead=head;};
	    void setReference(float raRef, float decRef){this->itsRAref=raRef;this->itsDECref=decRef;};
	    void setSizeFlag(bool b){itsFlagReportSize=b;};

	    void defineComponent(sourcefitting::RadioSource *src, size_t fitNum, std::string fitType="best");
	    void print(std::ostream &theStream);

	    float flux(){return itsFlux;};
	    std::string ID(){return itsID;};

	    friend std::ostream& operator<<(std::ostream &theStream, ParsetComponent &comp);
	    friend bool operator<(ParsetComponent &lhs, ParsetComponent &rhs){return lhs.itsFlux < rhs.itsFlux;};

	private:
	    duchamp::FitsHeader *itsHead;
	    float itsFlux;
	    float itsRAref;
	    float itsDECref;
	    float itsRAoff;
	    float itsDECoff;
	    bool  itsFlagReportSize;
	    float itsBmaj;
	    float itsBmin;
	    float itsBpa;
	    std::string itsID;
	};

    }

}




#endif
