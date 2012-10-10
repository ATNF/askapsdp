#include <outputs/AskapAsciiCatalogueWriter.h>
#include <askap_analysis.h>

#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>
#include <sourcefitting/FitResults.h>


namespace askap { 

  namespace analysis {

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter():
      duchamp::ASCIICatalogueWriter()
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(std::string name):
      duchamp::ASCIICatalogueWriter(name)
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(duchamp::Catalogues::DESTINATION dest):
      duchamp::ASCIICatalogueWriter(dest)
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(std::string name,duchamp::Catalogues::DESTINATION dest):
      duchamp::ASCIICatalogueWriter(name,dest)
    {
      this->itsFlagWriteFits = true;
      this->itsSourceList = 0;
      this->itsFitType = "best";
    }

    AskapAsciiCatalogueWriter::AskapAsciiCatalogueWriter(const AskapAsciiCatalogueWriter& other)
    {
      this->operator=(other);
    }

    AskapAsciiCatalogueWriter& AskapAsciiCatalogueWriter::operator= (const AskapAsciiCatalogueWriter& other)
    {
      if(this==&other) return *this;
      ((ASCIICatalogueWriter &) *this) = other;
      this->itsFlagWriteFits = other.itsFlagWriteFits;
      this->itsSourceList = other.itsSourceList;
      this->itsFitType = other.itsFitType;
      return *this;
    }

    void AskapAsciiCatalogueWriter::setup(DuchampParallel *finder)
    {
      this->CatalogueWriter::setup(finder->pCube());
      this->itsSourceList = finder->pSourceList();
    }
    
    void AskapAsciiCatalogueWriter::writeEntries()
    {
      if(this->itsFlagWriteFits){
	if(this->itsOpenFlag){
	  for(std::vector<sourcefitting::RadioSource>::iterator src=this->itsSourceList->begin(); 
	      src<this->itsSourceList->end(); src++)
	    this->writeEntry(&*src);
	}
      }
      else this->CatalogueWriter::writeEntries();
      
    }    

    void AskapAsciiCatalogueWriter::writeEntry(sourcefitting::RadioSource *source)
    {
      sourcefitting::FitResults results = source->fitResults(this->itsFitType);
      int suffixCtr = 0;
      char firstSuffix = 'a';

      if (!results.isGood() && !results.fitIsGuess()) { //if no fits were made, and we haven't got a guess stored
	float zero = 0.;
	itsColumnSpecification->column("NUM").printEntry(*this->itsStream, source->getID());
	itsColumnSpecification->column("NAME").printEntry(*this->itsStream, source->getName());
	itsColumnSpecification->column("RAJD").printEntry(*this->itsStream, source->getRA());
	itsColumnSpecification->column("DECJD").printEntry(*this->itsStream, source->getDec());
	itsColumnSpecification->column("FINT").printEntry(*this->itsStream, source->getIntegFlux());
	itsColumnSpecification->column("FPEAK").printEntry(*this->itsStream, source->getPeakFlux());
	itsColumnSpecification->column("FINTFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("FPEAKFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("MAJFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("MINFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("PAFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("MAJDECONV").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("MINDECONV").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("PADECONV").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("ALPHA").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("BETA").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("CHISQFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("RMSIMAGE").printEntry(*this->itsStream, source->noiseLevel());
	itsColumnSpecification->column("RMSFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("NFREE").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("NDOFFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("NPIXFIT").printEntry(*this->itsStream, zero);
	itsColumnSpecification->column("NPIXOBJ").printEntry(*this->itsStream, source->getSize());
	itsColumnSpecification->column("GUESS").printEntry(*this->itsStream, zero);
	*this->itsStream << "\n";
      } else {
	std::vector<casa::Gaussian2D<Double> > fitSet = results.fitSet();
	std::vector<casa::Gaussian2D<Double> >::iterator fit = fitSet.begin();
	std::vector<float> alphaVals = source->alphaValues(this->itsFitType);
	std::vector<float>::iterator alphaIter = alphaVals.begin();
	std::vector<float> betaVals = source->betaValues(this->itsFitType);
	std::vector<float>::iterator betaIter = betaVals.begin();
	ASKAPCHECK(fitSet.size() == alphaVals.size(),
		   "Sizes of fitSet (" << fitSet.size() << ") and alpha Set (" << alphaVals.size() << ") for fittype '" << this->itsFitType << "' don't match!");
	ASKAPCHECK(fitSet.size() == betaVals.size(),
		   "Sizes of fitSet (" << fitSet.size() << ") and beta Set (" << betaVals.size() << ") for fittype '" << this->itsFitType << "' don't match!");

	for (; fit < fitSet.end(); fit++, alphaIter++, betaIter++) {

	  std::vector<Double> deconv = deconvolveGaussian(*fit,this->itsHead->getBeam());
	  std::stringstream id;
	  id << source->getID() << char(firstSuffix + suffixCtr++);
	  double *pix = new double[3];
	  pix[0] = fit->xCenter();
	  pix[1] = fit->yCenter();
	  pix[2] = source->getZcentre();
	  double *wld = new double[3];
	  this->itsHead->pixToWCS(pix, wld);
	  double sourceRA = wld[0];
	  double sourceDec = wld[1];
	  delete [] pix;
	  delete [] wld;
	  float intfluxfit = fit->flux();

	  if (this->itsHead->needBeamSize())
	    intfluxfit /= this->itsHead->beam().area(); // Convert from Jy/beam to Jy
	  //                             intfluxfit /= this->itsHead->getBeamSize(); // Convert from Jy/beam to Jy

	  itsColumnSpecification->column("NUM").printEntry(*this->itsStream, id.str());
	  itsColumnSpecification->column("NAME").printEntry(*this->itsStream, source->getName());
	  itsColumnSpecification->column("RAJD").printEntry(*this->itsStream, sourceRA);
	  itsColumnSpecification->column("DECJD").printEntry(*this->itsStream, sourceDec);
	  itsColumnSpecification->column("FINT").printEntry(*this->itsStream, source->getIntegFlux());
	  itsColumnSpecification->column("FPEAK").printEntry(*this->itsStream, source->getPeakFlux());
	  itsColumnSpecification->column("FINTFIT").printEntry(*this->itsStream, intfluxfit);
	  itsColumnSpecification->column("FPEAKFIT").printEntry(*this->itsStream, fit->height());
	  itsColumnSpecification->column("MAJFIT").printEntry(*this->itsStream, fit->majorAxis()*this->itsHead->getAvPixScale()*3600.); // convert from pixels to arcsec
	  itsColumnSpecification->column("MINFIT").printEntry(*this->itsStream, fit->minorAxis()*this->itsHead->getAvPixScale()*3600.);
	  itsColumnSpecification->column("PAFIT").printEntry(*this->itsStream, fit->PA()*180. / M_PI);
	  itsColumnSpecification->column("MAJDECONV").printEntry(*this->itsStream, deconv[0]*this->itsHead->getAvPixScale()*3600.); // convert from pixels to arcsec
	  itsColumnSpecification->column("MINDECONV").printEntry(*this->itsStream, deconv[1]*this->itsHead->getAvPixScale()*3600.);
	  itsColumnSpecification->column("PADECONV").printEntry(*this->itsStream, deconv[2]*180. / M_PI);
	  itsColumnSpecification->column("ALPHA").printEntry(*this->itsStream, *alphaIter);
	  itsColumnSpecification->column("BETA").printEntry(*this->itsStream, *betaIter);
	  itsColumnSpecification->column("CHISQFIT").printEntry(*this->itsStream, results.chisq());
	  itsColumnSpecification->column("RMSIMAGE").printEntry(*this->itsStream, source->noiseLevel());
	  itsColumnSpecification->column("RMSFIT").printEntry(*this->itsStream, results.RMS());
	  itsColumnSpecification->column("NFREE").printEntry(*this->itsStream, results.numFreeParam());
	  itsColumnSpecification->column("NDOFFIT").printEntry(*this->itsStream, results.ndof());
	  itsColumnSpecification->column("NPIXFIT").printEntry(*this->itsStream, results.numPix());
	  itsColumnSpecification->column("NPIXOBJ").printEntry(*this->itsStream, source->getSize());
	  itsColumnSpecification->column("GUESS").printEntry(*this->itsStream, results.fitIsGuess() ? 1 : 0);
	  *this->itsStream << "\n";
	}
      }

    }

  }

}
