#include <conrad_conrad.h>
#include <askap/ConradLogging.h>

using namespace conrad;

int main() {
  // Now set its level. Normally you do not need to set the
  // level of a logger programmatically. This is usually done
  // in configuration files.
  CONRADLOG_INIT("tLogging.log_cfg");
  int i = 1;
  CONRAD_LOGGER(locallog, ".test");

  CONRADLOG_WARN(locallog,"Warning. This is a warning.");
  CONRADLOG_INFO(locallog,"This is an automatic (subpackage) log");
  CONRADLOG_INFO_STR(locallog,"This is " << i << " log stream test.");
  return 0;
}
