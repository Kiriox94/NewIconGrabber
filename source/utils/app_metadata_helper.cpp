#include "utils/app_metadata_helper.hpp"
#include <switch.h>
#include <sstream>
#include <string.h> 
#include <borealis/core/logger.hpp>

namespace appMetadataHelper
{
    std::vector<NxTitleCacheApplicationMetadata*> getInstalledGames() {
        std::vector<NxTitleCacheApplicationMetadata*> games;

        NsApplicationRecord* records = new NsApplicationRecord[64000]();
        int recordCount = 0;
        uint64_t tid;
        Result rc;

        rc = nsListApplicationRecord(records, 64000, 0, &recordCount);
        if (R_SUCCEEDED(rc)) {    
            for (auto i = 0; i < recordCount; ++i) {
                tid = records[i].application_id;
                NxTitleCacheApplicationMetadata* metadata = getMetadataFromTitleId(tid);
                if (metadata != nullptr) games.push_back(metadata);
            }
        }
        delete[] records;
        return games;
    }

    NxTitleCacheApplicationMetadata* getMetadataFromTitleId(u64 tid) {
        if (!nxtcCheckIfEntryExists(tid)) {
            NsApplicationControlData controlData;
            size_t controlSize = 0, iconSize = 0;

            if (R_FAILED(nsGetApplicationControlData(NsApplicationControlSource_Storage, tid, &controlData, sizeof(controlData), &controlSize))) {
                return nullptr;
            }
            iconSize = (controlSize - sizeof(NacpStruct));

            if (!nxtcAddEntry(tid, &controlData.nacp, iconSize, controlData.icon, false)) {
                return nullptr;
            }

            brls::Logger::info("Added {} ({}) to title cache", tid, controlData.nacp.lang[0].name);
        }

        NxTitleCacheApplicationMetadata* metadata = nxtcGetApplicationMetadataEntryById(tid);
        brls::Logger::info("{} fetched from title cache", metadata->name);
        return metadata;
    }
} // namespace appMetadataHelper
