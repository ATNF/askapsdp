/// @file AskapAsciiCatalogueWriter.h
///
/// Write out the fitted Gaussian components to a parset suitable for reading into csimulator & ccalibrator
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

#ifndef ASKAP_COMP_PARSET_WRITER_H_
#define ASKAP_COMP_PARSET_WRITER_H_

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>
#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>

namespace askap {

  namespace analysis { 

    class AskapComponentParsetWriter : public duchamp::ASCIICatalogueWriter
    {
    public:
	AskapComponentParsetWriter();
	AskapComponentParsetWriter(std::string name);
	AskapComponentParsetWriter(duchamp::Catalogues::DESTINATION dest);
	AskapComponentParsetWriter(std::string name, duchamp::Catalogues::DESTINATION dest);
	AskapComponentParsetWriter(const AskapComponentParsetWriter& other);
	AskapComponentParsetWriter& operator= (const AskapComponentParsetWriter& other);
	virtual ~AskapComponentParsetWriter(){};

	std::vector<sourcefitting::RadioSource> *sourcelist(){return itsSourceList;};
	void setSourceList(std::vector<sourcefitting::RadioSource> *srclist){itsSourceList = srclist;};
	std::string fitType(){return itsFitType;};
	void setFitType(std::string s){itsFitType = s;};
	float getRefRA(){return itsRefRA;};
	void setRefRA(float f){itsRefRA=f;};
	float getRefDec(){return itsRefDec;};
	void setRefDec(float f){itsRefDec=f;};
	bool flagReportSize(){return itsFlagReportSize;};
	void setFlagReportSize(bool b){itsFlagReportSize=b;};
	int maxNumComponents(){return itsMaxNumComponents;};
	void setMaxNumComponents(int i){itsMaxNumComponents = i;};

	void setup(duchamp::Cube *cube);
     	void writeTableHeader();
 	/// @brief Front-end to writing the catalogue. 
	void writeEntries();
	void writeFooter();

	// The following functions now do nothing.
	void writeHeader(){};
	void writeCommandLineEntry(int argc, char *argv[]){};
	void writeParameters(){};
	void writeStats(){};
	void writeEntry(duchamp::Detection *object){};///< redefine this so that it does nothing.
	void writeCubeSummary(){};

    protected:

	std::vector<sourcefitting::RadioSource> *itsSourceList;
	std::string itsFitType; ///< Which fit type to write out.
	float itsRefRA;
	float itsRefDec;
	bool itsFlagReportSize; ///< If true, show the fitted size, else give as point source.
	std::string itsSourceIDlist;  ///< List of all components added to parset
	int itsMaxNumComponents;
    };

  }

}
#endif
