#include <string.h>

#include "auto_update_policy.h"
#include "firmware_version.h"

namespace AutoUpdatePolicy
{
    uint8_t countStart(uint8_t starts) { return starts == UINT8_MAX ? starts : starts + 1; }

    bool isDue(uint8_t starts, bool onProbation) { return !onProbation && starts >= CHECK_EVERY_STARTS; }

    bool worthOffering(const char *found, const char *running, const char *rolledBack)
    {
        return FirmwareVersion::isNewer(found, running) && strcmp(found, rolledBack) != 0;
    }

    bool shouldAsk(const char *found, const char *running, const char *rolledBack, bool onProbation)
    {
        return !onProbation && worthOffering(found, running, rolledBack);
    }
}
