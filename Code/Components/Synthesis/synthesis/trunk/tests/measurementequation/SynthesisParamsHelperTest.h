/// @file
/// 
/// @brief Unit tests for SynthesisParamsHelper.
/// @details SynthesisParamsHelper class contains utilities to simplify
/// handling of parameters representing images. This unit test is intended to
/// test this functionality.
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

#ifndef SYNTHESIS_PARAMS_HELPER_TEST_H
#define SYNTHESIS_PARAMS_HELPER_TEST_H

#include <measurementequation/SynthesisParamsHelper.h>
#include <cppunit/extensions/HelperMacros.h>

#include <askap/AskapError.h>
#include <askap/AskapUtil.h>

#include <vector>
#include <map>
#include <string>

namespace askap
{
  namespace synthesis
  {
    
    class SynthesisParamsHelperTest : public CppUnit::TestFixture
    {
      CPPUNIT_TEST_SUITE(SynthesisParamsHelperTest);
      CPPUNIT_TEST(testListFacet);
      CPPUNIT_TEST(testFacetCreationAndMerging);
      CPPUNIT_TEST_SUITE_END();
      
      private:

      public:

        void testListFacet()
        {
           std::vector<std::string> names;
           std::map<std::string,int> facetmap;
           names.push_back("image.i.src.facet.0.0");
           names.push_back("image.i.src.facet.0.1");
           names.push_back("image.i.src.facet.1.0");
           names.push_back("image.i.src.facet.1.1");
           names.push_back("image.i.src2");
           SynthesisParamsHelper::listFacets(names,facetmap);
           CPPUNIT_ASSERT(facetmap.size()==2);
           CPPUNIT_ASSERT(facetmap.find("image.i.src")!=facetmap.end());
           CPPUNIT_ASSERT(facetmap["image.i.src"] == 2);
           CPPUNIT_ASSERT(facetmap.find("image.i.src2")!=facetmap.end());
           CPPUNIT_ASSERT(facetmap["image.i.src2"] == 1);           
        }
        
        void testFacetCreationAndMerging()
        {
           askap::scimath::Params params;
           std::vector<std::string> direction(3);
           direction[0]="12h30m00.0";
           direction[1]="-15.00.00.00";
           direction[2]="J2000";
           std::vector<int> shape(2,256);
           std::vector<std::string> cellsize(2,"8arcsec");
           SynthesisParamsHelper::add(params,"testsrc",direction,cellsize,shape,1.4e9,
                                      1.4e9,1,2,128);
           // checking the content
           std::map<std::string,int> facetmap;
           SynthesisParamsHelper::listFacets(params.freeNames(),facetmap);
           CPPUNIT_ASSERT(facetmap.find("testsrc")!=facetmap.end());
           CPPUNIT_ASSERT(facetmap["testsrc"] == 2);
           // adding a merged image
           SynthesisParamsHelper::add(params,"testsrc",2);
           params.fix("testsrc");
           CPPUNIT_ASSERT(params.freeNames().size() == 4); 
           CPPUNIT_ASSERT(params.names().size() == 5); 
           const std::vector<std::string> &facets = params.freeNames();
           for (std::vector<std::string>::const_iterator ci = facets.begin(); ci!=facets.end(); ++ci) {
                 CPPUNIT_ASSERT(SynthesisParamsHelper::getFacet(params,*ci).shape() == 
                          casa::IPosition(4,128,128,1,1));
           }
        }
   };
    
  } // namespace synthesis
} // namespace askap

#endif // #ifndef SYNTHESIS_PARAMS_HELPER_TEST_H

