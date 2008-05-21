#include <measurementequation/IImagePreconditioner.h>

namespace askap
{
  namespace synthesis
  {

    IImagePreconditioner::IImagePreconditioner() 
    {
    }
    
    IImagePreconditioner::~IImagePreconditioner() 
    {
    }
    
    bool IImagePreconditioner::doPreconditioning(casa::Array<float>& psf, casa::Array<float>& dirty) 
    {
      return false;
    }

  }
}


