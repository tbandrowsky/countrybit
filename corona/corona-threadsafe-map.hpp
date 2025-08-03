
#ifndef CORONA_THREAD_SAFE_MAP_H
#define CORONA_THREAD_SAFE_MAP_H

#include <map>
#include <mutex>


namespace corona {

template<typename K, typename V>
class thread_safe_map {
    std::map<K, V> data;
    mutable lockable locker;

public:
    thread_safe_map() = default;
    ~thread_safe_map() = default;

    void insert(const K& key, const V& value) {
        scope_lock lock(locker);
        data[key] = value;
    }

    bool try_get(const K& key, V& value) const {
        scope_lock lock(locker);
        auto it = data.find(key);
        if (it != data.end()) {
            value = it->second;
            return true;
        }
        return false;
    }

    bool erase(const K& key) {
        scope_lock lock(locker);
        auto it = data.find(key);
        if (it != data.end()) {
            data.erase(it);
        }
        return size() == 0;
    }

    size_t size() const {
        scope_lock lock(locker);
        return data.size();
    }

    void clear() {
        scope_lock lock(locker);
        data.clear();
    }
};

}

#endif
