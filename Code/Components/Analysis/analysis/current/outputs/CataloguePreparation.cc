/// @file
///
/// Code to prepare catalogues for output
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
#include <askap_analysis.h>
#include <askap/AskapLogging.h>
#include <askap/AskapError.h>
#include <outputs/CataloguePreparation.h>

#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FitResults.h>
#include <mathsutils/MathsUtils.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/Outputs/CatalogueSpecification.hh>

#include <string>
#include <vector>

///@brief Where the log messages go.
ASKAP_LOGGER(logger, ".catPrep");

using namespace duchamp::Catalogues;

namespace askap {

namespace analysis {

std::string getSuffix(unsigned int num)
{

    char initialLetter = 'a';
    std::stringstream id;
    for (unsigned int c = 0, count = 0, factor = 1;
            count <= num;
            factor *= 26, c++, count += factor) {
        int n = ((num - count) / factor) % 26;
        id << char(initialLetter + n);
    }
    std::string suff = id.str();
    std::reverse(suff.begin(), suff.end());

    return suff;
}

void getResultsParams(casa::Gaussian2D<Double> &gauss,
                      duchamp::FitsHeader &head, double zval,
                      std::vector<Double> &deconvShape, double &ra,
                      double &dec, double &intFluxFit)
{
    deconvShape = analysisutilities::deconvolveGaussian(gauss, head.getBeam());
    double zworld;
    if (head.pixToWCS(gauss.xCenter(), gauss.yCenter(), zval, ra, dec, zworld) != 0) {
        ASKAPLOG_ERROR_STR(logger, "Error with pixToWCS conversion");
    }
    intFluxFit = gauss.flux();
    if (head.needBeamSize()) {
        intFluxFit /= head.beam().area(); // Convert from Jy/beam to Jy
    }
}


CatalogueSpecification IslandCatalogue(duchamp::FitsHeader &header)
{
    const int prFlux = duchamp::Catalogues::prFLUX;
    const int prPos = duchamp::Catalogues::prPOS;
    const int prWpos = duchamp::Catalogues::prWPOS;
    const int prVel = duchamp::Catalogues::prVEL;
    const int prXYZ = duchamp::Catalogues::prXYZ;
    CatalogueSpecification newSpec;
    newSpec.addColumn("NUM", "island_id", "--", 6, 0,
                      "meta.id;meta.main", "char", "col_island_id", "");
    newSpec.addColumn("NAME", "island_name", "", 8, 0,
                      "meta.id", "char", "col_island_name", "");
    newSpec.addColumn("NCOMP", "n_components", "", 5, 0,
                      "meta.number", "int", "col_num_components", "");
    newSpec.addColumn("RA", "ra_hms_cont", "", 11, 0,
                      "pos.eq.ra", "char", "col_ra", "J2000");
    newSpec.addColumn("DEC", "dec_dms_cont", "", 11, 0,
                      "pos.eq.dec", "char", "col_dec", "J2000");
    newSpec.addColumn("RAJD", "ra_deg_cont", "[deg]", 11, prPos,
                      "pos.eq.ra;meta.main", "float", "col_rajd", "J2000");
    newSpec.addColumn("DECJD", "dec_deg_cont", "[deg]", 11, prPos,
                      "pos.eq.dec;meta.main", "float", "col_decjd", "J2000");
    newSpec.addColumn("VEL", "freq", "[MHz]", 11, prVel,
                      "em.freq", "float", "col_freq", "");
    newSpec.addColumn("MAJ", "maj_axis", "[arcsec]", 6, prWpos,
                      "phys.angSize.smajAxis;em.radio", "float", "col_maj", "");
    newSpec.addColumn("MIN", "min_axis", "[arcsec]", 6, prWpos,
                      "phys.angSize.sminAxis;em.radio", "float", "col_min", "");
    newSpec.addColumn("PA", "pos_ang", "[deg]", 7, prWpos,
                      "phys.angSize;pos.posAng;em.radio", "float", "col_pa", "");
    newSpec.addColumn("FINT", "flux_int", "[mJy]", 10, prFlux,
                      "phot.flux.density.integrated;em.radio", "float", "col_fint", "");
    newSpec.addColumn("FPEAK", "flux_peak", "[mJy/beam]", 9, prFlux,
                      "phot.flux.density;stat.max;em.radio", "float", "col_fpeak", "");
    newSpec.addColumn("X1", "x_min", "", 4, 0,
                      "pos.cartesian.x;stat.min", "int", "col_x1", "");
    newSpec.addColumn("X2", "x_max", "", 4, 0,
                      "pos.cartesian.x;stat.max", "int", "col_x2", "");
    newSpec.addColumn("Y1", "y_min", "", 4, 0,
                      "pos.cartesian.y;stat.min", "int", "col_y1", "");
    newSpec.addColumn("Y2", "y_max", "", 4, 0,
                      "pos.cartesian.y;stat.max", "int", "col_y2", "");
    newSpec.addColumn("SPATSIZE", "n_pix", "", 9, 0,
                      "phys.angArea;instr.pixel;meta.number", "int", "col_npix", "");
    newSpec.addColumn("XAV", "x_ave", "", 6, prXYZ,
                      "pos.cartesian.x;stat.mean", "float", "col_xav", "");
    newSpec.addColumn("YAV", "y_ave", "", 6, prXYZ,
                      "pos.cartesian.y;stat.mean", "float", "col_yav", "");
    newSpec.addColumn("XCENT", "x_cen", "", 7, prXYZ,
                      "pos.cartesian.x;askap:stat.centroid", "float", "col_xcent", "");
    newSpec.addColumn("YCENT", "y_cen", "", 7, prXYZ,
                      "pos.cartesian.y;askap:stat.centroid", "float", "col_ycent", "");
    newSpec.addColumn("XPEAK", "x_peak", "", 7, prXYZ,
                      "pos.cartesian.x;phot.flux;stat.max", "int", "col_xpeak", "");
    newSpec.addColumn("YPEAK", "y_peak", "", 7, prXYZ,
                      "pos.cartesian.y;phot.flux;stat.max", "int", "col_ypeak", "");
    newSpec.addColumn("FLAG1", "flag_c1", "", 5, 0,
                      "meta.code", "int", "col_flag1", "");
    newSpec.addColumn("FLAG2", "flag_c2", "", 5, 0,
                      "meta.code", "int", "col_flag2", "");
    newSpec.addColumn("FLAG3", "flag_c3", "", 5, 0,
                      "meta.code", "int", "col_flag3", "");
    newSpec.addColumn("FLAG4", "flag_c4", "", 5, 0,
                      "meta.code", "int", "col_flag4", "");
    newSpec.addColumn("COMMENT", "comment", "", 100, 0,
                      "meta.note", "char", "col_comment", "");
    return newSpec;

}


CatalogueSpecification ComponentCatalogue(duchamp::FitsHeader &header)
{
    // Returns a component catalogue spec consistent with CASDA specs
    const int prFlux = duchamp::Catalogues::prFLUX;
    const int prPos = duchamp::Catalogues::prPOS;
    const int prWpos = duchamp::Catalogues::prWPOS;
    const int prVel = duchamp::Catalogues::prVEL;
    CatalogueSpecification newSpec;
    newSpec.addColumn("ISLAND", "island_id", "--", 6, 0,
                      "meta.id.parent", "char", "col_island_id", "");
    newSpec.addColumn("NUM", "component_id", "--", 6, 0,
                      "meta.id;meta.main", "char", "col_component_id", "");
    newSpec.addColumn("NAME", "component_name", "", 8, 0,
                      "meta.id", "char", "col_component_name", "");
    newSpec.addColumn("RA", "ra_hms_cont", "", 11, 0,
                      "pos.eq.ra", "char", "col_ra", "J2000");
    newSpec.addColumn("DEC", "dec_dms_cont", "", 11, 0,
                      "pos.eq.dec", "char", "col_dec", "J2000");
    newSpec.addColumn("RAJD", "ra_deg_cont", "[deg]", 11, prPos,
                      "pos.eq.ra;meta.main", "float", "col_rajd", "J2000");
    newSpec.addColumn("DECJD", "dec_deg_cont", "[deg]", 11, prPos,
                      "pos.eq.dec;meta.main", "float", "col_decjd", "J2000");
    newSpec.addColumn("RAERR", "ra_err", "[deg]", 11, prPos,
                      "stat.error;pos.eq.ra", "float", "col_raerr", "J2000");
    newSpec.addColumn("DECERR", "dec_err", "[deg]", 11, prPos,
                      "stat.error;pos.eq.dec", "float", "col_decerr", "J2000");
    newSpec.addColumn("VEL", "freq", "[" + header.getSpectralUnits() + "]", 11, prVel,
                      "em.freq", "float", "col_freq", "");
    newSpec.addColumn("FPEAKFIT", "flux_peak", "[mJy/beam]",
                      9, prFlux,
                      "phot.flux.density;stat.max;em.radio;stat.fit",
                      "float", "col_fpeak", "");
    newSpec.addColumn("FPEAKFITERR", "flux_peak_err", "[mJy/beam]",
                      9, prFlux,
                      "stat.error;phot.flux.density;stat.max;em.radio;stat.fit",
                      "float", "col_fpeak_err", "");
    newSpec.addColumn("FINTFIT", "flux_int", "[mJy]",
                      9, prFlux,
                      "phot.flux.density;em.radio;stat.fit",
                      "float", "col_fint", "");
    newSpec.addColumn("FINTFITERR", "flux_int_err", "[mJy]",
                      9, prFlux,
                      "stat.error;phot.flux.density;em.radio;stat.fit",
                      "float", "col_fint_err", "");
    newSpec.addColumn("MAJFIT", "maj_axis", "[arcsec]", 6, prWpos,
                      "phys.angSize.smajAxis;em.radio;stat.fit",
                      "float", "col_maj", "");
    newSpec.addColumn("MINFIT", "min_axis", "[arcsec]", 6, prWpos,
                      "phys.angSize.sminAxis;em.radio;stat.fit",
                      "float", "col_min", "");
    newSpec.addColumn("PAFIT", "pos_ang", "[deg]", 7, prWpos,
                      "phys.angSize;pos.posAng;em.radio;stat.fit",
                      "float", "col_pa", "");
    newSpec.addColumn("MAJERR", "maj_axis_err", "[arcsec]", 6, prWpos,
                      "stat.error;phys.angSize.smajAxis;em.radio",
                      "float", "col_maj_err", "");
    newSpec.addColumn("MINERR", "min_axis_err", "[arcsec]", 6, prWpos,
                      "stat.error;phys.angSize.sminAxis;em.radio",
                      "float", "col_min_err", "");
    newSpec.addColumn("PAERR", "pos_ang_err", "[deg]", 7, prWpos,
                      "stat.error;phys.angSize;pos.posAng;em.radio",
                      "float", "col_pa_err", "");
    newSpec.addColumn("MAJDECONV", "maj_axis_deconv", "[arcsec]", 6, prWpos,
                      "phys.angSize.smajAxis;em.radio;askap:meta.deconvolved",
                      "float", "col_maj_deconv", "");
    newSpec.addColumn("MINDECONV", "min_axis_deconv", "[arcsec]", 6, prWpos,
                      "phys.angSize.sminAxis;em.radio;askap:meta.deconvolved",
                      "float", "col_min_deconv", "");
    newSpec.addColumn("PADECONV", "pos_ang_deconv", "[deg]", 7, prWpos,
                      "phys.angSize;pos.posAng;em.radio;askap:meta.deconvolved",
                      "float", "col_pa_deconv", "");
    newSpec.addColumn("CHISQFIT", "chi_squared_fit", "--", 10, 3,
                      "stat.fit.chi2", "float", "col_chisqfit", "");
    newSpec.addColumn("RMSFIT", "rms_fit_gauss", "[mJy/beam]", 10, 3,
                      "stat.stdev;stat.fit", "float", "col_rmsfit", "");
    newSpec.addColumn("ALPHA", "spectral_index", "--", 8, 3,
                      "spect.index;em.radio", "float", "col_alpha", "");
    newSpec.addColumn("BETA", "spectral_curvature", "--", 8, 3,
                      "askap:spect.curvature;em.radio", "float", "col_beta", "");
    newSpec.addColumn("RMSIMAGE", "rms_image", "[mJy/beam]", 10, 3,
                      "stat.stdev;phot.flux.density", "float", "col_rmsimage", "");
    newSpec.addColumn("FLAG1", "flag_c1", "", 5, 0,
                      "meta.code", "int", "col_flag1", "");
    newSpec.addColumn("FLAG2", "flag_c2", "", 5, 0,
                      "meta.code", "int", "col_flag2", "");
    newSpec.addColumn("FLAG3", "flag_c3", "", 5, 0,
                      "meta.code", "int", "col_flag3", "");
    newSpec.addColumn("FLAG4", "flag_c4", "", 5, 0,
                      "meta.code", "int", "col_flag4", "");
    newSpec.addColumn("COMMENT", "comment", "", 100, 0,
                      "meta.note", "char", "col_comment", "");

    return newSpec;

}


CatalogueSpecification fullCatalogue(CatalogueSpecification inputSpec,
                                     duchamp::FitsHeader &header)
{

    // /// @todo Make this a more obvious parameter to change
    // const int fluxPrec = 8;
    // const int fluxWidth = fluxPrec + 12;

    CatalogueSpecification newSpec;
    newSpec.addColumn(inputSpec.column("NUM"));
    newSpec.column("NUM").setName("ID");
    newSpec.column("NUM").setUnits("--");
    newSpec.column("NUM").setDatatype("char");
    newSpec.addColumn(inputSpec.column("NAME"));
    newSpec.column("NAME").setUnits("--");
    newSpec.addColumn(inputSpec.column("RAJD"));
    newSpec.addColumn(inputSpec.column("DECJD"));
    // newSpec.addColumn(inputSpec.column("VEL"));
    newSpec.addColumn(inputSpec.column("X"));
    newSpec.column("X").setUnits("[pix]");
    newSpec.addColumn(inputSpec.column("Y"));
    newSpec.column("Y").setUnits("[pix]");
    // newSpec.addColumn(inputSpec.column("Z"));
    newSpec.addColumn(inputSpec.column("FINT"));
    newSpec.addColumn(inputSpec.column("FPEAK"));

    newSpec.column("FINT").setUCD("phot.flux.density.integrated");
    // newSpec.column("FINT").changePrec(fluxPrec);
    newSpec.column("FPEAK").setUCD("phot.flux.density.peak");
    // newSpec.column("FPEAK").changePrec(fluxPrec);
    // new columns
    newSpec.addColumn("FINTFIT", "F_int(fit)", "[" + header.getIntFluxUnits() + "]",
                      10, 3,
                      "phot.flux.density.integrated;stat.fit",
                      "float", "col_fint_fit", "");
    newSpec.addColumn("FPEAKFIT", "F_pk(fit)", "[" + header.getFluxUnits() + "]",
                      10, 3,
                      "phot.flux.density.peak;stat.fit",
                      "float", "col_fpeak_fit", "");
    newSpec.addColumn("MAJFIT", "Maj(fit)", "[arcsec]", 10, 3,
                      "phys.angSize.smajAxis", "float", "col_maj_fit", "");
    newSpec.addColumn("MINFIT", "Min(fit)", "[arcsec]", 10, 3,
                      "phys.angSize.sminAxis", "float", "col_min_fit", "");
    newSpec.addColumn("PAFIT", "P.A.(fit)", "[deg]", 10, 2,
                      "phys.angSize;pos.posAng", "float", "col_pa_fit", "");
    newSpec.addColumn("MAJDECONV", "Maj(fit_deconv.)", "[arcsec]", 17, 3,
                      "phys.angSize.smajAxis;meta.deconvolved",
                      "float", "col_maj_deconv", "");
    newSpec.addColumn("MINDECONV", "Min(fit_deconv.)", "[arcsec]", 17, 3,
                      "phys.angSize.sminAxis;meta.deconvolved",
                      "float", "col_min_deconv", "");
    newSpec.addColumn("PADECONV", "P.A.(fit_deconv.)", "[deg]", 18, 2,
                      "phys.angSize;pos.posAng;meta.deconvolved",
                      "float", "col_pa_deconv", "");
    newSpec.addColumn("ALPHA", "Alpha", "--", 8, 3,
                      "spect.index", "float", "col_alpha", "");
    newSpec.addColumn("BETA", "Beta", "--", 8, 3,
                      "spect.curvature", "float", "col_beta", "");
    newSpec.addColumn("CHISQFIT", "Chisq(fit)", "--", 10, 3,
                      "stat.fit.chi2", "float", "col_chisqfit", "");
    newSpec.addColumn("RMSIMAGE", "RMS(image)", "[" + header.getFluxUnits() + "]",
                      10, 3, "stat.stdev;phot.flux.density",
                      "float", "col_rmsimage", "");
    newSpec.addColumn("RMSFIT", "RMS(fit)", "[" + header.getFluxUnits() + "]",
                      10, 3, "stat.stdev;stat.fit", "float", "col_rmsfit", "");
    newSpec.addColumn("NFREEFIT", "Nfree(fit)", "--", 11, 0,
                      "meta.number;stat.fit.param;stat.fit",
                      "int", "col_nfreefit", "");
    newSpec.addColumn("NDOFFIT", "NDoF(fit)", "--", 10, 0,
                      "stat.fit.dof", "int", "col_ndoffit", "");
    newSpec.addColumn("NPIXFIT", "NPix(fit)", "--", 10, 0,
                      "meta.number;instr.pixel", "int", "col_npixfit", "");
    newSpec.addColumn("NPIXOBJ", "NPix(obj)", "--", 10, 0,
                      "meta.number;instr.pixel;stat.fit",
                      "int", "col_npixobj", "");
    newSpec.addColumn("GUESS", "Guess?", "--", 7, 0,
                      "meta.flag", "int", "col_guess", "");

    return newSpec;
}


void setupCols(CatalogueSpecification &spec,
               std::vector<sourcefitting::RadioSource> &srclist,
               std::string fitType)
{

    std::vector<sourcefitting::RadioSource>::iterator src;
    for (src = srclist.begin(); src != srclist.end(); src++) {
//        src->addOffsets();
        sourcefitting::FitResults results = src->fitResults(fitType);
        for (unsigned int n = 0; n < results.numFits(); n++) {
            casa::Gaussian2D<Double> gauss = results.gaussian(n);
            std::vector<Double> deconvShape;
            double ra, dec, intFluxFit;
            getResultsParams(gauss, src->header(), src->getZcentre(),
                             deconvShape, ra, dec, intFluxFit);
            float cdelt = src->header().WCS().cdelt[src->header().WCS().lng];
            int precision = -int(log10(fabs(cdelt * 3600. / 10.)));
            std::string raS  = decToDMS(ra, src->header().lngtype(), precision);
            std::string decS = decToDMS(dec, src->header().lattype(), precision);
            std::string name = src->header().getIAUName(ra, dec);
            spec.column("ISLAND").check(src->getID());
            std::stringstream compid;
            compid << src->getID() << getSuffix(n);
            spec.column("NUM").check(compid.str());
            spec.column("NAME").check(src->getName());
            spec.column("RA").check(raS);
            spec.column("DEC").check(decS);
            spec.column("RAJD").check(ra);
            spec.column("DECJD").check(dec);
            spec.column("RAERR").check(0.);
            spec.column("DECERR").check(0.);
            spec.column("X").check(gauss.xCenter());
            spec.column("Y").check(gauss.yCenter());
            spec.column("FINT").check(src->getIntegFlux());
            spec.column("FPEAK").check(src->getPeakFlux());
            spec.column("FINTFIT").check(intFluxFit);
            spec.column("FINTFITERR").check(0.);
            spec.column("FPEAKFIT").check(gauss.height());
            spec.column("FPEAKFITERR").check(0.);
            float pixScale = src->header().getAvPixScale() * 3600.; //arcsec/pix
            spec.column("MAJFIT").check(gauss.majorAxis()*pixScale);
            spec.column("MINFIT").check(gauss.minorAxis()*pixScale);
            spec.column("PAFIT").check(gauss.PA() * 180. / M_PI, false);
            spec.column("MAJERR").check(0.);
            spec.column("MINERR").check(0.);
            spec.column("PAERR").check(0.);
            spec.column("MAJDECONV").check(deconvShape[0]*pixScale);
            spec.column("MINDECONV").check(deconvShape[1]*pixScale);
            spec.column("PADECONV").check(deconvShape[2] * 180. / M_PI, false);
            spec.column("ALPHA").check(src->alphaValues(fitType)[n]);
            spec.column("BETA").check(src->betaValues(fitType)[n]);
            spec.column("CHISQFIT").check(results.chisq());
            spec.column("RMSIMAGE").check(src->noiseLevel());
            spec.column("RMSFIT").check(results.RMS());
            spec.column("NFREEFIT").check(results.numFreeParam());
            spec.column("NDOFFIT").check(results.ndof());
            spec.column("NPIXFIT").check(results.numPix());
            spec.column("NPIXOBJ").check(src->getSize());
            spec.column("FLAG1").check(1);
            spec.column("FLAG2").check(1);
            spec.column("FLAG3").check(1);
            spec.column("FLAG4").check(1);
            spec.column("COMMENT").check("");
        }
    }
}



}

}
