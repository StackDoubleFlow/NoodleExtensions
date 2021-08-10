#pragma once

#include <vector>

class NESharedUpdate {
    static inline std::vector<void (*)()> callbacks;

public:
    static void AddCallback(void (*callback)()) {
        callbacks.push_back(callback);
    }

    static void TriggerUpdate() {
        for (auto callback : callbacks) {
            callback();
        }
    }
};
