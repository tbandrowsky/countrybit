#pragma once

#include <map>
#include <mutex>

namespace corona {

template<typename K, typename V>
class thread_safe_map {
    std::map<K, V> data;
    mutable std::mutex mtx;

public:
    thread_safe_map() = default;
    ~thread_safe_map() = default;

    void insert(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mtx);
        data[key] = value;
    }

    bool try_get(const K& key, V& value) const {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = data.find(key);
        if (it != data.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    void erase(const K& key) {
        std::lock_guard<std::mutex> lock(mtx);
        data.erase(key);
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data.size();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        data.clear();
    }
};

}