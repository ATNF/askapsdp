#include <conrad_synthesis.h>
#include <conrad/ConradLogging.h>
CONRAD_LOGGER(logger, ".gridding");

#include <conrad/ConradError.h>
#include <gridding/VisGridderFactory.h>
#include <gridding/BoxVisGridder.h>
#include <gridding/SphFuncVisGridder.h>
#include <gridding/WProjectVisGridder.h>
#include <gridding/AWProjectVisGridder.h>
#include <gridding/WStackVisGridder.h>
#include <gridding/AProjectWStackVisGridder.h>

using namespace LOFAR::ACC::APS;
namespace conrad
{
  namespace synthesis
  {

    VisGridderFactory::VisGridderFactory()
    {
    }

    VisGridderFactory::~VisGridderFactory()
    {
    }

    IVisGridder::ShPtr VisGridderFactory::make(
        const LOFAR::ACC::APS::ParameterSet &parset)
    {
      IVisGridder::ShPtr gridder;
      /// @todo Better handling of string case
      if (parset.getString("gridder")=="WProject")
      {
        double wmax=parset.getDouble("gridder.WProject.wmax", 35000.0);
        int nwplanes=parset.getInt32("gridder.WProject.nwplanes", 64);
        double cutoff=parset.getDouble("gridder.WProject.cutoff", 1e-3);
        int oversample=parset.getInt32("gridder.WProject.oversample", 8);
        int maxSupport=parset.getInt32("gridder.WProject.maxsupport", 256);
        string tablename=parset.getString("gridder.WProject.tablename", "");
        CONRADLOG_INFO_STR(logger, "Gridding using W projection");
        gridder=IVisGridder::ShPtr(new WProjectVisGridder(wmax, nwplanes, cutoff, oversample,
            maxSupport, tablename));
      }
      else if (parset.getString("gridder")=="WStack")
      {
        double wmax=parset.getDouble("gridder.WStack.wmax", 35000.0);
        int nwplanes=parset.getInt32("gridder.WStack.nwplanes", 64);
        CONRADLOG_INFO_STR(logger, "Gridding using W stacking ");
        gridder=IVisGridder::ShPtr(new WStackVisGridder(wmax, nwplanes));
      }
      else if (parset.getString("gridder")=="AWProject")
      {
        double diameter=parset.getDouble("gridder.AWProject.diameter");
        double blockage=parset.getDouble("gridder.AWProject.blockage");
        double wmax=parset.getDouble("gridder.AWProject.wmax", 10000.0);
        int nwplanes=parset.getInt32("gridder.AWProject.nwplanes", 64);
        double cutoff=parset.getDouble("gridder.AWProject.cutoff", 1e-3);
        int oversample=parset.getInt32("gridder.AWProject.oversample", 8);
        int maxSupport=parset.getInt32("gridder.AWProject.maxsupport", 128);
        bool freqDep=parset.getBool("gridder.AWProject.frequencydependent", true);
        int maxFeeds=parset.getInt32("gridder.AWProject.maxfeeds", 1);
        string tablename=parset.getString("gridder.AWProject.tablename", "");
        CONRADLOG_INFO_STR(logger,  "Gridding with Using Antenna Illumination and W projection"
                                    );
        if(freqDep) {
          CONRADLOG_INFO_STR(logger, "Antenna illumination scales with frequency" );
        }
        else {
          CONRADLOG_INFO_STR(logger, "Antenna illumination independent of frequency" );
        }
        gridder=IVisGridder::ShPtr(new AWProjectVisGridder(diameter, blockage,
            wmax, nwplanes, cutoff, oversample,
            maxSupport, maxFeeds, freqDep, tablename));
      }
      else if (parset.getString("gridder")=="AProjectWStack")
      {
        double diameter=parset.getDouble("gridder.AProjectWStack.diameter");
        double blockage=parset.getDouble("gridder.AProjectWStack.blockage");
        double wmax=parset.getDouble("gridder.AProjectWStack.wmax", 10000.0);
        int nwplanes=parset.getInt32("gridder.AProjectWStack.nwplanes", 64);
        int oversample=parset.getInt32("gridder.AProjectWStack.oversample", 8);
        int maxSupport=
            parset.getInt32("gridder.AProjectWStack.maxsupport", 128);
        int maxFeeds=parset.getInt32("gridder.AProjectWStack.maxfeeds", 1);
        bool freqDep=parset.getBool("gridder.AProjectWStack.frequencydependent", true);
        string tablename=parset.getString("gridder.AProjectWStack.tablename",
            "");
        CONRADLOG_INFO_STR(logger, "Gridding with Antenna Illumination projection and W stacking"
                                    );
        if(freqDep) {
          CONRADLOG_INFO_STR(logger, "Antenna illumination scales with frequency" );
        }
        else {
          CONRADLOG_INFO_STR(logger, "Antenna illumination independent of frequency" );
        }
        gridder=IVisGridder::ShPtr(new AProjectWStackVisGridder(diameter, blockage,
            wmax, nwplanes, oversample,
            maxSupport, maxFeeds, freqDep, tablename));
      }
      else if (parset.getString("gridder")=="Box")
      {
        CONRADLOG_INFO_STR(logger, "Gridding with Box function");
        gridder=IVisGridder::ShPtr(new BoxVisGridder());
      }
      else
      {
        CONRADLOG_INFO_STR(logger, "Gridding with spheriodal function");
        gridder=IVisGridder::ShPtr(new SphFuncVisGridder());
      }
      return gridder;
    }
  }
}
