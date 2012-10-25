/// @file VOTableTest.cc
///
/// @copyright (c) 2012 CSIRO
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

// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

// Support classes
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>

// Classes to test
#include "votable/VOTable.h"

using namespace std;

namespace askap {
namespace accessors {

class VOTableTest : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(VOTableTest);
        CPPUNIT_TEST(testDescription);
        CPPUNIT_TEST(testXML);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        };

        void tearDown() {
        }

        void testDescription() {
            const string desc = "Test Description";
            VOTable vot;
            vot.setDescription(desc);
            CPPUNIT_ASSERT(vot.getDescription() == desc);
        }

        void testXML() {
            // Create a test VOTable
            const VOTable vot1 = makeTable();

            // Convert to XML
            std::stringstream ss;
            vot1.toXML(ss);
            //writeStringstream(ss);

            // Convert XML back to VOTable
            ss.seekg(0, ios::beg);
            const VOTable vot2 = VOTable::fromXML(ss);

            // Verify VOTable
            CPPUNIT_ASSERT(vot2.getDescription() == vot1.getDescription());
            CPPUNIT_ASSERT_EQUAL(1ul, vot2.getResource().size());

            // Verify Info
            CPPUNIT_ASSERT_EQUAL(0ul, vot2.getInfo().size());

            // Verify Resource
            const VOTableResource res1 = vot1.getResource()[0];
            const VOTableResource res2 = vot2.getResource()[0];
            CPPUNIT_ASSERT_EQUAL(1ul, res2.getTables().size());

            const VOTableTable vottab1 = res1.getTables()[0];
            const VOTableTable vottab2 = res2.getTables()[0];
            CPPUNIT_ASSERT(vottab1.getName() == vottab2.getName());
            CPPUNIT_ASSERT(vottab1.getDescription() == vottab2.getDescription());

            // Verify groups
            const std::vector<VOTableGroup> groups = vottab2.getGroups();
            CPPUNIT_ASSERT_EQUAL(1ul, groups.size());
            CPPUNIT_ASSERT_EQUAL(2ul, groups[0].getFieldRefs().size());
            const std::vector<VOTableParam> params = groups[0].getParams();
            CPPUNIT_ASSERT_EQUAL(1ul, params.size());
            CPPUNIT_ASSERT(params[0].getDatatype() == "char");
            CPPUNIT_ASSERT(params[0].getArraysize() == "*");
            CPPUNIT_ASSERT(params[0].getUCD() == "pos.frame");
            CPPUNIT_ASSERT(params[0].getName() == "cooframe");
            CPPUNIT_ASSERT(params[0].getUType() == "stc:AstroCoords.coord_system_id");
            CPPUNIT_ASSERT(params[0].getValue() == "UTC-ICRS-TOPO");

            // Verify fields
            const std::vector<VOTableField> fields = vottab2.getFields();
            CPPUNIT_ASSERT_EQUAL(2ul, fields.size());
            CPPUNIT_ASSERT(fields[0].getName() == "RA");
            CPPUNIT_ASSERT(fields[0].getID() == "col1");
            CPPUNIT_ASSERT(fields[0].getUCD() == "pos.eq.ra;meta.main");
            CPPUNIT_ASSERT(fields[0].getRef() == "J2000");
            CPPUNIT_ASSERT(fields[0].getUType() == "stc:AstroCoords.Position2D.Value2.C1");
            CPPUNIT_ASSERT(fields[0].getDatatype() == "float");
            CPPUNIT_ASSERT(fields[0].getUnit() == "deg");
            CPPUNIT_ASSERT(fields[1].getName() == "Dec");
            CPPUNIT_ASSERT(fields[1].getID() == "col2");
            CPPUNIT_ASSERT(fields[1].getUCD() == "pos.eq.dec;meta.main");
            CPPUNIT_ASSERT(fields[1].getRef() == "J2000");
            CPPUNIT_ASSERT(fields[1].getUType() == "stc:AstroCoords.Position2D.Value2.C2");
            CPPUNIT_ASSERT(fields[1].getDatatype() == "float");
            CPPUNIT_ASSERT(fields[1].getUnit() == "deg");

            // Verify rows
            const std::vector<VOTableRow> rows = vottab2.getRows();
            CPPUNIT_ASSERT_EQUAL(2ul, fields.size());
            CPPUNIT_ASSERT(rows[0].getCells()[0] == "1.0");
            CPPUNIT_ASSERT(rows[0].getCells()[1] == "2.0");
            CPPUNIT_ASSERT(rows[1].getCells()[0] == "3.0");
            CPPUNIT_ASSERT(rows[1].getCells()[1] == "4.0");
        }

    private:
        static void writeStringstream(const std::stringstream& ss) {
            std::ofstream fs("unittest_votable.xml", fstream::out | fstream::trunc);
            CPPUNIT_ASSERT(fs);
            fs << ss.str();
            fs.close();
        }

        static VOTable makeTable() {
            VOTableTable vottab;
            vottab.setName("tabletablename");
            vottab.setDescription("tabletabledesc");

            // Add group
            {
                VOTableGroup grp;
                grp.setID("J2000");
                grp.setUType("stc:AstroCoords");
                {
                    VOTableParam p;
                    p.setDatatype("char");
                    p.setArraysize("*");
                    p.setUCD("pos.frame");
                    p.setName("cooframe");
                    p.setUType("stc:AstroCoords.coord_system_id");
                    p.setValue("UTC-ICRS-TOPO");
                    grp.addParam(p);
                }
                grp.addFieldRef("col1");
                grp.addFieldRef("col2");
                vottab.addGroup(grp);
            }

            // Add fields
            {
                // RA
                VOTableField f;
                f.setName("RA");
                f.setID("col1");
                f.setUCD("pos.eq.ra;meta.main");
                f.setRef("J2000");
                f.setUType("stc:AstroCoords.Position2D.Value2.C1");
                f.setDatatype("float");
                f.setUnit("deg");
                vottab.addField(f);
            }

            {
                // Dec
                VOTableField f;
                f.setName("Dec");
                f.setID("col2");
                f.setUCD("pos.eq.dec;meta.main");
                f.setRef("J2000");
                f.setUType("stc:AstroCoords.Position2D.Value2.C2");
                f.setDatatype("float");
                f.setUnit("deg");
                vottab.addField(f);
            }

            // Add rows
            {
                VOTableRow row;
                row.addCell("1.0");
                row.addCell("2.0");
                vottab.addRow(row);
            }

            {
                VOTableRow row;
                row.addCell("3.0");
                row.addCell("4.0");
                vottab.addRow(row);
            }

            VOTableResource res;
            res.setName("Test Resource");
            res.addTable(vottab);

            VOTable vot;
            vot.setDescription("Test Description");
            vot.addResource(res);

            return vot;
        }
};

}
}
