#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>

#include <wcslib/wcs.h>

#include <duchamp/fitsHeader.hh>
#include <duchamp/param.hh>
#include <duchamp/Cubes/cubes.hh>

#include <casa/aipstype.h>
#include <images/Images/ImageInterface.h>
using namespace casa;

namespace askap
{

  namespace analysis
  {

    void getCasaImage(std::string imageName);
    void storeWCStoHeader(duchamp::FitsHeader &head, duchamp::Param &par, wcsprm *wcs);
    int casaImageToCube(duchamp::Cube &cube);
    int casaImageToMetadata(duchamp::Cube &cube);
    int casaImageToMetadata(ImageInterface<Float> *imagePtr, duchamp::Cube &cube);
    int casaImageToCubeData(ImageInterface<Float> *imagePtr, duchamp::Cube &cube);
    void readBeamInfo(ImageInterface<Float>* imagePtr, duchamp::FitsHeader &head, duchamp::Param &par);
    wcsprm *casaImageToWCS(std::string imageName);
    wcsprm *casaImageToWCS(ImageInterface<Float>* imagePtr);

  }

}
