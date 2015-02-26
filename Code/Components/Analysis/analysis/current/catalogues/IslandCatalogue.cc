/// @file
///
/// Defining an Island Catalogue
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
#include <catalogues/IslandCatalogue.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <catalogues/casda.h>
#include <catalogues/CasdaIsland.h>
#include <sourcefitting/RadioSource.h>
#include <outputs/AskapVOTableCatalogueWriter.h>
#include <outputs/AskapAsciiCatalogueWriter.h>
#include <duchampinterface/DuchampInterface.h>

#include <Common/ParameterSet.h>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <vector>

ASKAP_LOGGER(logger, ".islandcatalogue");

namespace askap {

namespace analysis {

IslandCatalogue::IslandCatalogue(std::vector<sourcefitting::RadioSource> &srclist,
                                 const LOFAR::ParameterSet &parset,
                                 duchamp::Cube &cube):
    itsIslands(),
    itsSpec(),
    itsCube(cube),
    itsVersion("casda.continuum_island_description_v0.5")
{
    this->defineIslands(srclist, parset);
    this->defineSpec();

    duchamp::Param par = parseParset(parset);
    std::string filenameBase = par.getOutFile();
    filenameBase.replace(filenameBase.rfind(".txt"),
                         std::string::npos, ".islands");
    itsVotableFilename = filenameBase + ".xml";
    itsAsciiFilename = filenameBase + ".txt";

}

void IslandCatalogue::defineIslands(std::vector<sourcefitting::RadioSource> &srclist,
                                    const LOFAR::ParameterSet &parset)
{
    std::vector<sourcefitting::RadioSource>::iterator src;
    for (src = srclist.begin(); src != srclist.end(); src++) {
        CasdaIsland island(*src, parset);
        itsIslands.push_back(island);
    }
}

void IslandCatalogue::defineSpec()
{
    itsSpec.addColumn("ID", "island_id", "--", 6, 0,
                      "meta.id;meta.main", "char", "col_island_id", "");
    itsSpec.addColumn("NAME", "island_name", "", 8, 0,
                      "meta.id", "char", "col_island_name", "");
    itsSpec.addColumn("NCOMP", "n_components", "", 5, 0,
                      "meta.number", "int", "col_num_components", "");
    itsSpec.addColumn("RA", "ra_hms_cont", "", 11, 0,
                      "pos.eq.ra", "char", "col_ra", "J2000");
    itsSpec.addColumn("DEC", "dec_dms_cont", "", 11, 0,
                      "pos.eq.dec", "char", "col_dec", "J2000");
    itsSpec.addColumn("RAJD", "ra_deg_cont", "[deg]", 11, casda::precPos,
                      "pos.eq.ra;meta.main", "float", "col_rajd", "J2000");
    itsSpec.addColumn("DECJD", "dec_deg_cont", "[deg]", 11, casda::precPos,
                      "pos.eq.dec;meta.main", "float", "col_decjd", "J2000");
    itsSpec.addColumn("FREQ", "freq", "[MHz]", 11, casda::precFreq,
                      "em.freq", "float", "col_freq", "");
    itsSpec.addColumn("MAJ", "maj_axis", "[arcsec]", 6, casda::precSize,
                      "phys.angSize.smajAxis;em.radio", "float", "col_maj", "");
    itsSpec.addColumn("MIN", "min_axis", "[arcsec]", 6, casda::precSize,
                      "phys.angSize.sminAxis;em.radio", "float", "col_min", "");
    itsSpec.addColumn("PA", "pos_ang", "[deg]", 7, casda::precSize,
                      "phys.angSize;pos.posAng;em.radio", "float", "col_pa", "");
    itsSpec.addColumn("FINT", "flux_int", "[mJy]", 10, casda::precFlux,
                      "phot.flux.density.integrated;em.radio", "float", "col_fint", "");
    itsSpec.addColumn("FPEAK", "flux_peak", "[mJy/beam]", 9, casda::precFlux,
                      "phot.flux.density;stat.max;em.radio", "float", "col_fpeak", "");
    itsSpec.addColumn("XMIN", "x_min", "", 4, 0,
                      "pos.cartesian.x;stat.min", "int", "col_x1", "");
    itsSpec.addColumn("XMAX", "x_max", "", 4, 0,
                      "pos.cartesian.x;stat.max", "int", "col_x2", "");
    itsSpec.addColumn("YMIN", "y_min", "", 4, 0,
                      "pos.cartesian.y;stat.min", "int", "col_y1", "");
    itsSpec.addColumn("YMAX", "y_max", "", 4, 0,
                      "pos.cartesian.y;stat.max", "int", "col_y2", "");
    itsSpec.addColumn("NPIX", "n_pix", "", 9, 0,
                      "phys.angArea;instr.pixel;meta.number", "int", "col_npix", "");
    itsSpec.addColumn("XAV", "x_ave", "", 6, casda::precPix,
                      "pos.cartesian.x;stat.mean", "float", "col_xav", "");
    itsSpec.addColumn("YAV", "y_ave", "", 6, casda::precPix,
                      "pos.cartesian.y;stat.mean", "float", "col_yav", "");
    itsSpec.addColumn("XCENT", "x_cen", "", 7, casda::precPix,
                      "pos.cartesian.x;askap:stat.centroid", "float", "col_xcent", "");
    itsSpec.addColumn("YCENT", "y_cen", "", 7, casda::precPix,
                      "pos.cartesian.y;askap:stat.centroid", "float", "col_ycent", "");
    itsSpec.addColumn("XPEAK", "x_peak", "", 7, casda::precPix,
                      "pos.cartesian.x;phot.flux;stat.max", "int", "col_xpeak", "");
    itsSpec.addColumn("YPEAK", "y_peak", "", 7, casda::precPix,
                      "pos.cartesian.y;phot.flux;stat.max", "int", "col_ypeak", "");
    itsSpec.addColumn("FLAG1", "flag_i1", "", 5, 0,
                      "meta.code", "int", "col_flag1", "");
    itsSpec.addColumn("FLAG2", "flag_i2", "", 5, 0,
                      "meta.code", "int", "col_flag2", "");
    itsSpec.addColumn("FLAG3", "flag_i3", "", 5, 0,
                      "meta.code", "int", "col_flag3", "");
    itsSpec.addColumn("FLAG4", "flag_i4", "", 5, 0,
                      "meta.code", "int", "col_flag4", "");
    itsSpec.addColumn("COMMENT", "comment", "", 100, 0,
                      "meta.note", "char", "col_comment", "");

}

void IslandCatalogue::check()
{
    std::vector<CasdaIsland>::iterator isle;
    for (isle = itsIslands.begin(); isle != itsIslands.end(); isle++) {
        isle->checkSpec(itsSpec);
    }

}

void IslandCatalogue::write()
{
    this->writeVOT();
    this->writeASCII();
}

void IslandCatalogue::writeVOT()
{
    AskapVOTableCatalogueWriter vowriter(itsVotableFilename);
    vowriter.setup(&itsCube);
    ASKAPLOG_DEBUG_STR(logger, "Writing island table to the VOTable " <<
                       itsVotableFilename);
    vowriter.setColumnSpec(&itsSpec);
    vowriter.openCatalogue();
    vowriter.setResourceName("Island catalogue from Selavy source-finding");
    vowriter.setTableName("Island catalogue");
    vowriter.writeHeader();
    duchamp::VOParam version("table_version", "meta.version", "char", itsVersion, 39, "");
    vowriter.writeParameter(version);
    vowriter.writeParameters();
    vowriter.writeStats();
    vowriter.writeTableHeader();
    vowriter.writeEntries<CasdaIsland>(itsIslands);
    vowriter.writeFooter();
    vowriter.closeCatalogue();
}

void IslandCatalogue::writeASCII()
{

    AskapAsciiCatalogueWriter writer(itsAsciiFilename);
    ASKAPLOG_DEBUG_STR(logger, "Writing Fit results to " << itsAsciiFilename);
    writer.setup(&itsCube);
    writer.setColumnSpec(&itsSpec);
    writer.openCatalogue();
    writer.writeTableHeader();
    writer.writeEntries<CasdaIsland>(itsIslands);
    writer.writeFooter();
    writer.closeCatalogue();

}



}

}
