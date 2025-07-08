#include <nxtc.h>
#include <vector>

namespace appMetadataHelper
{
    std::vector<NxTitleCacheApplicationMetadata*> getInstalledGames();
    NxTitleCacheApplicationMetadata* getMetadataFromTitleId(u64 tid);
} // namespace appMetadataHelper
