#include <askap_analysis.h>

#include <askap/AskapLogging.h>
#include <askap/AskapError.h>

#include <string>

#include <wcslib/wcs.h>

namespace askap
{

  namespace analysis
  {

    wcsprm *casaImageToWCS(std::string imageName);

  }

}
