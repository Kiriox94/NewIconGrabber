#pragma once
#include "BS_thread_pool.hpp"

namespace ThreadsManager {
    inline BS::thread_pool<0>& getImagesPool() {
        static BS::thread_pool<0> pool(6);
        return pool;
    }

    inline BS::thread_pool<0>& getRequestsPool() {
        static BS::thread_pool<0> pool(2);
        return pool;
    }
}