#include <list>
#include <stdexcept>
#include <vector>

template<typename KeyT, typename ValueT>
class Iterator
{
private:
    using vect_iter = typename std::vector<std::list<std::pair<const KeyT, ValueT>>>::iterator;
    using list_iter = typename std::list<std::pair<const KeyT, ValueT>>::iterator;

public:
    Iterator() = default;

    Iterator(vect_iter start, vect_iter finish):
        vect_it(start),
        vect_end(finish)
    {
        while ((vect_it != vect_end) && (vect_it->begin() == vect_it->end()))
            ++vect_it;

        if (vect_it != vect_end)
            list_it = vect_it->begin();
    }

    Iterator& operator++()
    {
        if (next(list_it) != vect_it->end())
        {
            ++list_it;

            return *this;
        }

        ++vect_it;

        while ((vect_it != vect_end) && (vect_it->begin() == vect_it->end()))
            ++vect_it;

        if (vect_it != vect_end)
            list_it = vect_it->begin();

        return *this;
    }

    Iterator operator++(int)
    {
        Iterator tmp(*this);

        ++(*this);

        return tmp;
    }

    bool operator==(const Iterator& other) const
    {
        if ((vect_it == vect_end) && (vect_it == other.vect_it))
            return true;

        return (vect_it == other.vect_it) && (list_it == other.list_it);
    }

    bool operator!=(const Iterator& other) const
    {
        return !((*this) == other);
    }

    auto operator*()
    {
        return *list_it;
    }

    auto operator->()
    {
        return &(*list_it);
    }

private:
    vect_iter vect_it;
    vect_iter vect_end;

    list_iter list_it;
};

template<typename KeyT, typename ValueT>
class CIterator
{
private:
    using vect_citer = typename std::vector<std::list<std::pair<const KeyT, ValueT>>>::const_iterator;
    using list_citer = typename std::list<std::pair<const KeyT, ValueT>>::const_iterator;

public:
    CIterator() = default;

    CIterator(vect_citer start, vect_citer finish):
        vect_it(start),
        vect_end(finish)
    {
        while ((vect_it != vect_end) && (vect_it->begin() == vect_it->end()))
            ++vect_it;

        if (vect_it != vect_end)
            list_it = vect_it->begin();
    }

    CIterator& operator++()
    {
        if (next(list_it) != vect_it->end())
        {
            ++list_it;

            return *this;
        }

        ++vect_it;

        while ((vect_it != vect_end) && (vect_it->begin() == vect_it->end()))
            ++vect_it;

        if (vect_it != vect_end)
            list_it = vect_it->begin();

        return *this;
    }

    CIterator operator++(int)
    {
        CIterator tmp(*this);

        ++(*this);

        return tmp;
    }

    bool operator==(const CIterator& other) const
    {
        if ((vect_it == vect_end) && (vect_it == other.vect_it))
            return true;

        return (vect_it == other.vect_it) && (list_it == other.list_it);
    }

    bool operator!=(const CIterator& other) const
    {
        return !((*this) == other);
    }

    auto operator*()
    {
        return *list_it;
    }

    auto operator->()
    {
        return &(*list_it);
    }

private:
    vect_citer vect_it;
    vect_citer vect_end;

    list_citer list_it;
};

template<typename KeyT, typename ValueT, typename HashT=std::hash<KeyT>>
class HashMap
{
public:
    using iterator = Iterator<KeyT, ValueT>;
    using const_iterator = CIterator<KeyT, ValueT>;

    HashMap(HashT tmp = HashT()):
        buckets(default_sz), sz(0), hasher(tmp)
    {
    }

    template<typename IterT>
    HashMap(IterT start, IterT finish, HashT tmp = HashT()):
        buckets(default_sz), sz(0), hasher(tmp)
    {
        copy_range(start, finish);
    }

    HashMap(const std::initializer_list<std::pair<KeyT, ValueT>>& buffer, HashT tmp = HashT()):
        buckets(default_sz), sz(0), hasher(tmp)
    {
        copy_range(buffer.begin(), buffer.end());
    }

    HashMap& operator=(const HashMap& from)
    {
        if (this == &from)
            return *this;

        clear();
        hasher = from.hasher;

        for (const auto &it : from)
            insert(it);

        return *this;
    }

    void insert(const std::pair<KeyT, ValueT>& element)
    {
        size_t id = get_hash(element.first);

        for (const auto &it : buckets[id])
        {
            if (it.first == element.first)
                return;
        }

        buckets[id].push_front(element);
        sz++;
        resize_table();
    }

    void erase(const KeyT& key)
    {
        size_t id = get_hash(key);

        for (auto it = buckets[id].begin(); it != buckets[id].end(); ++it)
        {
            if (it->first == key)
            {
                buckets[id].erase(it);
                sz--;
                resize_table();

                return;
            }
        }
    }

    iterator find(const KeyT& key)
    {
        size_t id = get_hash(key);

        for (auto &it : buckets[id])
        {
            if (it.first == key)
            {
                auto iter = iterator(buckets.begin() + id, buckets.end());

                while (!(iter->first == key))
                    ++iter;

                return iter;
            }
        }

        return end();
    }

    const_iterator find(const KeyT& key) const
    {
        size_t id = get_hash(key);

        for (const auto &it : buckets[id])
        {
            if (it.first == key)
            {
                auto iter = const_iterator(buckets.cbegin() + id, buckets.cend());

                while (!(iter->first == key))
                    ++iter;

                return iter;
            }
        }

        return end();
    }

    ValueT& operator[](const KeyT& key)
    {
        size_t id = get_hash(key);

        for (auto &it : buckets[id])
        {
            if (it.first == key)
                return it.second;
        }

        buckets[id].push_front(std::make_pair(key, ValueT()));
        sz++;
        resize_table();

        id = get_hash(key);

        for (auto &it : buckets[id])
        {
            if (it.first == key)
                return it.second;
        }
    }

    const ValueT& at(const KeyT& key) const
    {
        size_t id = get_hash(key);

        for (const auto &it : buckets[id])
        {
            if (it.first == key)
                return it.second;
        }

        throw std::out_of_range("at: not found key");
    }

    size_t size() const
    {
        return sz;
    }

    bool empty() const
    {
        return (sz == 0);
    }

    void clear()
    {
        buckets.clear();
        buckets.resize(default_sz);

        sz = 0;
    }

    HashT hash_function() const
    {
        return hasher;
    }

    iterator begin()
    {
        return iterator(buckets.begin(), buckets.end());
    }

    const_iterator begin() const
    {
        return const_iterator(buckets.cbegin(), buckets.cend());
    }

    iterator end()
    {
        return iterator(buckets.end(), buckets.end());
    }

    const_iterator end() const
    {
        return const_iterator(buckets.cend(), buckets.cend());
    }

private:
    using container_t = std::vector<std::list<std::pair<const KeyT, ValueT>>>;

    static const size_t default_sz = 1;

    container_t buckets;

    size_t sz;

    HashT hasher;

    size_t get_hash(const KeyT& key) const
    {
        return hasher(key) % buckets.size();
    }

    void resize_table()
    {
        if ((buckets.size() >= sz * 2) && (buckets.size() <= sz * 4))
            return;

        size_t new_sz = 0;

        if (buckets.size() < sz * 2)
            new_sz = buckets.size() * 2;

        if (buckets.size() > sz * 4)
            new_sz = buckets.size() / 2;

        container_t tmp(std::move(buckets));

        clear();
        buckets.resize(new_sz);

        for (size_t i = 0; i < tmp.size(); i++)
        {
            for (const auto &it : tmp[i])
            {
                size_t id = get_hash(it.first);

                buckets[id].push_front(it);

                sz++;
            }
        }
    }

    template<typename IterT>
    void copy_range(IterT start, IterT finish)
    {
        for (auto it = start; it != finish; ++it)
            insert(*it);
    }
};