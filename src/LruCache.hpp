#include <list>
#include <optional>
#include <tuple>
#include <unordered_map>

template <typename KeyT, typename ValueT> class LruCache {
public:
    LruCache(size_t max_size)
        : m_max_size{max_size} {}

    void put(const KeyT& key, const ValueT& value) {
        auto it = m_cache.find(key);

        m_lru.emplace_front(key, value);

        if (it != m_cache.end()) {
            m_lru.erase(it->second);
            m_cache.erase(it);
        }

        m_cache.emplace(key, m_lru.begin());

        if (m_lru.size() > m_max_size) {
            auto last = m_lru.rbegin();
            m_cache.erase(std::get<0>(*last));
            m_lru.pop_back();
        }
    }

    std::optional<std::reference_wrapper<const ValueT>> get(const KeyT& key) {
        auto it = m_cache.find(key);

        if (it == m_cache.end()) {
            return std::nullopt;
        }

        m_lru.splice(m_lru.begin(), m_lru, it->second);

        return std::cref(std::get<1>(*(it->second)));
    }

    auto has(const KeyT& key) { return m_cache.find(key) != m_cache.end(); }

    auto size() const { return m_cache.size(); }

private:
    using KeyValue = std::tuple<KeyT, ValueT>;
    using ListIterator = typename std::list<KeyValue>::iterator;

    std::list<KeyValue> m_lru{};
    std::unordered_map<KeyT, ListIterator> m_cache{};
    size_t m_max_size{};
};