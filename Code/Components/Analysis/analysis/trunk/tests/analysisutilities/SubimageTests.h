/// @file
///
/// XXX Notes on program XXX
///
/// @copyright (c) 2008 CSIRO
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
/// @author Matthew Whiting <matthew.whiting@csiro.au>
///
#include <analysisutilities/SubimageDef.h>
#include <cppunit/extensions/HelperMacros.h>
#include <wcslib/wcs.h>
#include <APS/ParameterSet.h>

#include <string>
#include <math.h>

namespace askap {

  namespace analysis {

    class SubimageTest : public CppUnit::TestFixture {
      CPPUNIT_TEST_SUITE(SubimageTest);
      CPPUNIT_TEST(fullFieldSingle);
      CPPUNIT_TEST(fullFieldQuarterNoOverlap);
      CPPUNIT_TEST(fullFieldQuarterOverlap);
      CPPUNIT_TEST(subsectionQuarterOverlap);
      CPPUNIT_TEST_SUITE_END();

    private:

	// Define a WCS struct. Just needed for definition of SubimageDefs, and only need number of axes and which is which.
	struct wcsprm *dummyWCS; 

	SubimageDef subdef;
	LOFAR::ACC::APS::ParameterSet parset; // used for defining the subdef
	std::string baseSection;
	std::vector<long> imageDim;

    public:

      void setUp() {
	
	imageDim = std::vector<long>(4);
	imageDim[0] = imageDim[1] = 100;
	imageDim[2] = imageDim[3] = 1;
	
	dummyWCS = (struct wcsprm *)calloc(1, sizeof(struct wcsprm));
	dummyWCS->flag = -1;
	dummyWCS->naxis=4;
	dummyWCS->lng=0;
	dummyWCS->lat=1;
	dummyWCS->spec=3;

	parset.add("image","testimage");
	parset.add("nsubx","1");
	parset.add("nsuby","1");
	parset.add("nsubz","1");
	parset.add("overlapx","0");
	parset.add("overlapy","0");
	parset.add("overlapz","0");

      }

      void tearDown(){
	int nwcs=1;
	wcsvfree(&nwcs,&dummyWCS);
      };

      void fullFieldSingle(){
	baseSection = "[*,*,*,*]";
	subdef = SubimageDef(parset);
	subdef.setImageDim(imageDim);
	subdef.define(dummyWCS);
	CPPUNIT_ASSERT( subdef.section(-1,baseSection).getSection()==baseSection );
      }

      void fullFieldQuarterNoOverlap(){
	baseSection = "[*,*,*,*]";
	parset.replace("nsubx","2");
	parset.replace("nsuby","2");
	subdef = SubimageDef(parset);
	subdef.setImageDim(imageDim);
	subdef.define(dummyWCS);
	CPPUNIT_ASSERT( subdef.section(-1,baseSection).getSection()==baseSection );
	CPPUNIT_ASSERT( subdef.section(0,baseSection).getSection()=="[1:50,1:50,*,*]" );
	CPPUNIT_ASSERT( subdef.section(1,baseSection).getSection()=="[51:100,1:50,*,*]" );
	CPPUNIT_ASSERT( subdef.section(2,baseSection).getSection()=="[1:50,51:100,*,*]" );
	CPPUNIT_ASSERT( subdef.section(3,baseSection).getSection()=="[51:100,51:100,*,*]" );
      }

      void fullFieldQuarterOverlap(){
	baseSection = "[*,*,*,*]";
	parset.replace("nsubx","2");
	parset.replace("nsuby","2");
	parset.replace("overlapx","10");
	parset.replace("overlapy","10");
	subdef = SubimageDef(parset);
	subdef.setImageDim(imageDim);
	subdef.define(dummyWCS);
	CPPUNIT_ASSERT( subdef.section(-1,baseSection).getSection()==baseSection );
	CPPUNIT_ASSERT( subdef.section(0,baseSection).getSection()=="[1:55,1:55,*,*]" );
	CPPUNIT_ASSERT( subdef.section(1,baseSection).getSection()=="[46:100,1:55,*,*]" );
	CPPUNIT_ASSERT( subdef.section(2,baseSection).getSection()=="[1:55,46:100,*,*]" );
	CPPUNIT_ASSERT( subdef.section(3,baseSection).getSection()=="[46:100,46:100,*,*]" );
      }

      void subsectionQuarterOverlap(){
	baseSection = "[26:75,31:90,*,*]";
	parset.replace("nsubx","2");
	parset.replace("nsuby","2");
	parset.replace("overlapx","10");
	parset.replace("overlapy","10");
	subdef = SubimageDef(parset);
	subdef.setImageDim(imageDim);
	subdef.define(dummyWCS);
	CPPUNIT_ASSERT( subdef.section(-1,baseSection).getSection()==baseSection );
	CPPUNIT_ASSERT( subdef.section(0,baseSection).getSection()=="[26:55,31:65,*,*]" );
	CPPUNIT_ASSERT( subdef.section(1,baseSection).getSection()=="[46:75,31:65,*,*]" );
	CPPUNIT_ASSERT( subdef.section(2,baseSection).getSection()=="[26:55,56:90,*,*]" );
	CPPUNIT_ASSERT( subdef.section(3,baseSection).getSection()=="[46:75,56:90,*,*]" );
      }

    };

  }

}
