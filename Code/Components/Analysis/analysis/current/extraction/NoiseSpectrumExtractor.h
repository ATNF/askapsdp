#ifndef ASKAP_ANALYSIS_EXTRACTION_NOISESPECTRUM_H_
#define ASKAP_ANALYSIS_EXTRACTION_NOISESPECTRUM_H_
#include <askap_analysis.h>
#include <extraction/SourceDataExtractor.h>
#include <extraction/SpectralBoxExtractor.h>
#include <Common/ParameterSet.h>

namespace askap {

  namespace analysis {

    class NoiseSpectrumExtractor : public SpectralBoxExtractor
    {
    public:
      NoiseSpectrumExtractor(){};
      NoiseSpectrumExtractor(const LOFAR::ParameterSet& parset);
      virtual ~NoiseSpectrumExtractor(){};
      NoiseSpectrumExtractor(const NoiseSpectrumExtractor& other);
      NoiseSpectrumExtractor& operator=(const NoiseSpectrumExtractor& other);
      
      void setBoxWidth(int w){itsBoxWidth=w; if(this->itsSource) define();};
      void setBoxWidth();
      void setBoxAreaInBeams(float a){itsAreaInBeams=a; setBoxWidth();};
      float boxArea(){return itsAreaInBeams;};

      void extract();

    protected:
      float itsAreaInBeams;

    };


  }

}


#endif
