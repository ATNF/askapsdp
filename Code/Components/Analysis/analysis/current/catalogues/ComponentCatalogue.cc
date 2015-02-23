/// @file
///
/// Defining an Component Catalogue
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
#include <catalogues/ComponentCatalogue.h>
#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <catalogues/CasdaComponent.h>
#include <catalogues/casda.h>

#include <sourcefitting/RadioSource.h>
#include <outputs/AskapVOTableCatalogueWriter.h>
#include <outputs/AskapAsciiCatalogueWriter.h>
#include <duchampinterface/DuchampInterface.h>

#include <Common/ParameterSet.h>
#include <duchamp/Outputs/CatalogueSpecification.hh>
#include <vector>

ASKAP_LOGGER(logger, ".componentcatalogue");

namespace askap {

namespace analysis {

ComponentCatalogue::ComponentCatalogue(std::vector<sourcefitting::RadioSource> &srclist,
                                       const LOFAR::ParameterSet &parset,
                                       duchamp::Cube &cube):
    itsComponents(),
    itsSpec(),
    itsCube(cube),
    itsVersion("casda.continuum_component_description_v1.7")
{
    this->defineComponents(srclist, parset);
    this->defineSpec();

    duchamp::Param par = parseParset(parset);
    std::string filenameBase = par.getOutFile();
    filenameBase.replace(filenameBase.rfind(".txt"),
                         std::string::npos, ".components");
    itsVotableFilename = filenameBase + ".xml";
    itsAsciiFilename = filenameBase + ".txt";

}

void ComponentCatalogue::defineComponents(std::vector<sourcefitting::RadioSource> &srclist,
        const LOFAR::ParameterSet &parset)
{
    std::vector<sourcefitting::RadioSource>::iterator src;
    for (src = srclist.begin(); src != srclist.end(); src++) {
        for (size_t i = 0; i < src->numFits(); i++) {
            CasdaComponent component(*src, parset, i);
            itsComponents.push_back(component);
        }
    }
}

void ComponentCatalogue::defineSpec()
{
    itsSpec.addColumn("ISLAND", "island_id", "--", 6, 0,
                      "meta.id.parent", "char", "col_island_id", "");
    itsSpec.addColumn("ID", "component_id", "--", 6, 0,
                      "meta.id;meta.main", "char", "col_component_id", "");
    itsSpec.addColumn("NAME", "component_name", "", 8, 0,
                      "meta.id", "char", "col_component_name", "");
    itsSpec.addColumn("RA", "ra_hms_cont", "", 11, 0,
                      "pos.eq.ra", "char", "col_ra", "J2000");
    itsSpec.addColumn("DEC", "dec_dms_cont", "", 11, 0,
                      "pos.eq.dec", "char", "col_dec", "J2000");
    itsSpec.addColumn("RAJD", "ra_deg_cont", "[deg]", 11, casda::precPos,
                      "pos.eq.ra;meta.main", "float", "col_rajd", "J2000");
    itsSpec.addColumn("DECJD", "dec_deg_cont", "[deg]", 11, casda::precPos,
                      "pos.eq.dec;meta.main", "float", "col_decjd", "J2000");
    itsSpec.addColumn("RAERR", "ra_err", "[arcsec]", 11, casda::precSize,
                      "stat.error;pos.eq.ra", "float", "col_raerr", "J2000");
    itsSpec.addColumn("DECERR", "dec_err", "[arcsec]", 11, casda::precSize,
                      "stat.error;pos.eq.dec", "float", "col_decerr", "J2000");
    itsSpec.addColumn("FREQ", "freq", "[" + casda::freqUnit + "]", 11, casda::precFreq,
                      "em.freq", "float", "col_freq", "");
    itsSpec.addColumn("FPEAK", "flux_peak", "[" + casda::fluxUnit + "]",
                      9, casda::precFlux,
                      "phot.flux.density;stat.max;em.radio;stat.fit",
                      "float", "col_fpeak", "");
    itsSpec.addColumn("FPEAKERR", "flux_peak_err", "[" + casda::fluxUnit + "]",
                      9, casda::precFlux,
                      "stat.error;phot.flux.density;stat.max;em.radio;stat.fit",
                      "float", "col_fpeak_err", "");
    itsSpec.addColumn("FINT", "flux_int", "[" + casda::intFluxUnit + "]",
                      9, casda::precFlux,
                      "phot.flux.density;em.radio;stat.fit",
                      "float", "col_fint", "");
    itsSpec.addColumn("FINTERR", "flux_int_err", "[" + casda::intFluxUnit + "]",
                      9, casda::precFlux,
                      "stat.error;phot.flux.density;em.radio;stat.fit",
                      "float", "col_fint_err", "");
    itsSpec.addColumn("MAJ", "maj_axis", "[arcsec]", 6, casda::precSize,
                      "phys.angSize.smajAxis;em.radio;stat.fit",
                      "float", "col_maj", "");
    itsSpec.addColumn("MIN", "min_axis", "[arcsec]", 6, casda::precSize,
                      "phys.angSize.sminAxis;em.radio;stat.fit",
                      "float", "col_min", "");
    itsSpec.addColumn("PA", "pos_ang", "[deg]", 7, casda::precSize,
                      "phys.angSize;pos.posAng;em.radio;stat.fit",
                      "float", "col_pa", "");
    itsSpec.addColumn("MAJERR", "maj_axis_err", "[arcsec]", 6, casda::precSize,
                      "stat.error;phys.angSize.smajAxis;em.radio",
                      "float", "col_maj_err", "");
    itsSpec.addColumn("MINERR", "min_axis_err", "[arcsec]", 6, casda::precSize,
                      "stat.error;phys.angSize.sminAxis;em.radio",
                      "float", "col_min_err", "");
    itsSpec.addColumn("PAERR", "pos_ang_err", "[deg]", 7, casda::precSize,
                      "stat.error;phys.angSize;pos.posAng;em.radio",
                      "float", "col_pa_err", "");
    itsSpec.addColumn("MAJDECONV", "maj_axis_deconv", "[arcsec]", 6, casda::precSize,
                      "phys.angSize.smajAxis;em.radio;askap:meta.deconvolved",
                      "float", "col_maj_deconv", "");
    itsSpec.addColumn("MINDECONV", "min_axis_deconv", "[arcsec]", 6, casda::precSize,
                      "phys.angSize.sminAxis;em.radio;askap:meta.deconvolved",
                      "float", "col_min_deconv", "");
    itsSpec.addColumn("PADECONV", "pos_ang_deconv", "[deg]", 7, casda::precSize,
                      "phys.angSize;pos.posAng;em.radio;askap:meta.deconvolved",
                      "float", "col_pa_deconv", "");
    itsSpec.addColumn("CHISQ", "chi_squared_fit", "--", 10, casda::precFlux,
                      "stat.fit.chi2", "float", "col_chisqfit", "");
    itsSpec.addColumn("RMSFIT", "rms_fit_gauss", "[" + casda::fluxUnit + "]", 10, casda::precFlux,
                      "stat.stdev;stat.fit", "float", "col_rmsfit", "");
    itsSpec.addColumn("ALPHA", "spectral_index", "--", 8, casda::precSpec,
                      "spect.index;em.radio", "float", "col_alpha", "");
    itsSpec.addColumn("BETA", "spectral_curvature", "--", 8, casda::precSpec,
                      "askap:spect.curvature;em.radio", "float", "col_beta", "");
    itsSpec.addColumn("RMSIMAGE", "rms_image", "[" + casda::fluxUnit + "]", 10, casda::precFlux,
                      "stat.stdev;phot.flux.density", "float", "col_rmsimage", "");
    itsSpec.addColumn("FLAG1", "flag_c1", "", 5, 0,
                      "meta.code", "int", "col_flag1", "");
    itsSpec.addColumn("FLAG2", "flag_c2", "", 5, 0,
                      "meta.code", "int", "col_flag2", "");
    itsSpec.addColumn("FLAG3", "flag_c3", "", 5, 0,
                      "meta.code", "int", "col_flag3", "");
    itsSpec.addColumn("FLAG4", "flag_c4", "", 5, 0,
                      "meta.code", "int", "col_flag4", "");
    itsSpec.addColumn("COMMENT", "comment", "", 100, 0,
                      "meta.note", "char", "col_comment", "");

}

void ComponentCatalogue::check()
{
    std::vector<CasdaComponent>::iterator comp;
    for (comp = itsComponents.begin(); comp != itsComponents.end(); comp++) {
        comp->checkSpec(itsSpec);
    }

}

void ComponentCatalogue::write()
{
    this->writeVOT();
    this->writeASCII();
}

void ComponentCatalogue::writeVOT()
{
    AskapVOTableCatalogueWriter vowriter(itsVotableFilename);
    vowriter.setup(&itsCube);
    ASKAPLOG_DEBUG_STR(logger, "Writing component table to the VOTable " <<
                       itsVotableFilename);
    vowriter.setColumnSpec(&itsSpec);
    vowriter.openCatalogue();
    vowriter.writeHeader();
    duchamp::VOParam version("table_version", "meta.version", "char", itsVersion, 39, "");
    vowriter.writeParameter(version);
    vowriter.writeParameters();
    vowriter.writeFrequencyParam();
    vowriter.writeStats();
    vowriter.writeTableHeader();
    vowriter.writeEntries<CasdaComponent>(itsComponents);
    vowriter.writeFooter();
    vowriter.closeCatalogue();
}

void ComponentCatalogue::writeASCII()
{

    AskapAsciiCatalogueWriter writer(itsAsciiFilename);
    ASKAPLOG_DEBUG_STR(logger, "Writing Fit results to " << itsAsciiFilename);
    writer.setup(&itsCube);
    writer.setColumnSpec(&itsSpec);
    writer.openCatalogue();
    writer.writeTableHeader();
    writer.writeEntries<CasdaComponent>(itsComponents);
    writer.writeFooter();
    writer.closeCatalogue();

}



}

}
