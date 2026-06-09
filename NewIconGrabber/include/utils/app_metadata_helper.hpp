#ifdef __SWITCH__
#include <cstdlib>
#include <nxtc.h>
#include <vector>
#include <optional>
#include <string>

namespace appMetadataHelper
{
    std::vector<NxTitleCacheApplicationMetadata*> getInstalledGames();
    NxTitleCacheApplicationMetadata* getMetadataFromTitleId(u64 tid);
    std::optional<u64> extractTitleIDFromString(const std::string& input);
    std::string formatApplicationId(u64 ApplicationId);
} // namespace appMetadataHelper
#endif
