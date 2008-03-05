#include <askap_askap.h>
#include <askap/AskapLogging.h>

using namespace askap;

int main() {
  // Now set its level. Normally you do not need to set the
  // level of a logger programmatically. This is usually done
  // in configuration files.
  ASKAPLOG_INIT("tLogging.log_cfg");
  int i = 1;
  ASKAP_LOGGER(locallog, ".test");

  ASKAPLOG_WARN(locallog,"Warning. This is a warning.");
  ASKAPLOG_INFO(locallog,"This is an automatic (subpackage) log");
  ASKAPLOG_INFO_STR(locallog,"This is " << i << " log stream test.");
  return 0;
}
