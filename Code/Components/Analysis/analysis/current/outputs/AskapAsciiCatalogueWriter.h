#ifndef ASKAP_ASCII_CAT_WRITER_H_
#define ASKAP_ASCII_CAT_WRITER_H_

#include <duchamp/Outputs/ASCIICatalogueWriter.hh>
#include <parallelanalysis/DuchampParallel.h>
#include <sourcefitting/RadioSource.h>

namespace askap {

  namespace analysis { 

    class AskapAsciiCatalogueWriter : public duchamp::ASCIICatalogueWriter
    {
    public:
      AskapAsciiCatalogueWriter();
      AskapAsciiCatalogueWriter(std::string name);
      AskapAsciiCatalogueWriter(duchamp::Catalogues::DESTINATION dest);
      AskapAsciiCatalogueWriter(std::string name, duchamp::Catalogues::DESTINATION dest);
      AskapAsciiCatalogueWriter(const AskapAsciiCatalogueWriter& other);
      AskapAsciiCatalogueWriter& operator= (const AskapAsciiCatalogueWriter& other);
      virtual ~AskapAsciiCatalogueWriter(){};

      bool writeFits(){return itsFlagWriteFits;};
      void setFlagWriteFits(bool b){itsFlagWriteFits = b;};
      std::vector<sourcefitting::RadioSource> *sourcelist(){return itsSourceList;};
      void setSourceList(std::vector<sourcefitting::RadioSource> *srclist){itsSourceList = srclist;};
      std::string fitType(){return itsFitType;};
      void setFitType(std::string s){itsFitType = s;};

      void setup(DuchampParallel *finder);
      void writeTableHeader();
      void writeEntries();
      void writeEntry(sourcefitting::RadioSource *source);
      
    protected:
      bool itsFlagWriteFits; ///< Do we write the information on the fits to each source?
      std::vector<sourcefitting::RadioSource> *itsSourceList;
      std::string itsFitType; ///< Which fit type to write out.
    };

  }

}

#endif
