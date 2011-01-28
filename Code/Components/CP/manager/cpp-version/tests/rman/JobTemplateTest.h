/// @file JobTemplateTest.cc
///
/// @copyright (c) 2011 CSIRO
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
/// @author Ben Humphreys <ben.humphreys@csiro.au>

// System includes
#include <memory>
#include <map>
#include <string>

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include "askap/AskapError.h"
#include "rman/QJob.h"

// Classes to test
#include "rman/JobTemplate.h"

using namespace std;
using askap::cp::manager::JobTemplate;

namespace askap {
namespace cp {
namespace manager {

class JobTemplateTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(JobTemplateTest);
        CPPUNIT_TEST(testName);
        CPPUNIT_TEST(testScriptLocation);
        CPPUNIT_TEST(testAddDependency);
        CPPUNIT_TEST(testRemoveDependency);
        CPPUNIT_TEST(testGetDependencies);
        CPPUNIT_TEST(testRemoveAllDependencies);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            itsInstance.reset(new JobTemplate(""));
        };

        void tearDown() {
            itsInstance.reset();
        }

        void testName() {
            const string testVal = "testjob";
            itsInstance->setName(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, itsInstance->getName());
        }

        void testScriptLocation() {
            const string testVal = "/path/to/software/script.qsub";
            itsInstance->setScriptLocation(testVal);
            CPPUNIT_ASSERT_EQUAL(testVal, itsInstance->getScriptLocation());
        }

        void testAddDependency() {
            // Just check no exceptions are thrown
            QJob j1("1");
            itsInstance->addDependency(j1, JobTemplate::AFTERSTART);

            QJob j2("2");
            itsInstance->addDependency(j2, JobTemplate::AFTEROK);
        }

        void testRemoveDependency() {
            // Just check no exceptions are thrown
            QJob j1("1");
            itsInstance->addDependency(j1, JobTemplate::AFTERSTART);

            itsInstance->removeDependency(j1);
        }

        void testGetDependencies() {
            QJob j1("1");
            itsInstance->addDependency(j1, JobTemplate::AFTERSTART);

            QJob j2("2");
            itsInstance->addDependency(j2, JobTemplate::AFTEROK);

            map<std::string, JobTemplate::DependType> deps = itsInstance->getDependencies();
            CPPUNIT_ASSERT_EQUAL(2ul, deps.size());

            CPPUNIT_ASSERT(deps.find(j1.getId()) != deps.end());
            CPPUNIT_ASSERT(deps.find(j2.getId()) != deps.end());
        }

        void testRemoveAllDependencies() {
            QJob j1("1");
           itsInstance-> addDependency(j1, JobTemplate::AFTERSTART);

            QJob j2("2");
            itsInstance->addDependency(j2, JobTemplate::AFTEROK);

            map<std::string, JobTemplate::DependType> deps = itsInstance->getDependencies();
            CPPUNIT_ASSERT_EQUAL(2ul, deps.size());

            itsInstance->removeAllDependencies();

            deps = itsInstance->getDependencies();
            CPPUNIT_ASSERT_EQUAL(0ul, deps.size());
        }

    private:

        std::auto_ptr<JobTemplate> itsInstance;
};

}   // End namespace manager
}   // End namespace cp
}   // End namespace askap
