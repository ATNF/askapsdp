#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>

#include <wcslib/wcs.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>

#include <casa/aipstype.h>
#include <images/Images/ImageInterface.h>
using namespace casa;

namespace askap
{

  namespace analysis
  {

    void getCasaImage(std::string imageName);
    void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs);
    wcsprm *casaImageToWCS(std::string imageName);
    wcsprm *casaImageToWCS(ImageInterface<Float>* imagePtr);

  }

}
