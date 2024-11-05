#include <Basic/LStorage.hpp>

using namespace Basic;

template <typename T>
bool LStorage<T>::Register(const std::string &uniqueID, const T &item)
{
    auto target = directory.find(uniqueID);
    if (target != directory.end())
    {
        target->second = item;
        return false;
    }

    directory[uniqueID] = item;
    return true;
}

template <typename T>
bool LStorage<T>::Unregister(const std::string &uniqueID)
{
    auto target = directory.find(uniqueID);
    if (target == directory.end())
        return false;

    directory.erase(target);
    return true;
}

template <typename T>
T *LStorage<T>::GetPtr(const std::string &uniqueID)
{
    auto target = directory.find(uniqueID);
    if (target == directory.end())
        return NULL;

    return &(target->second);
}

template <typename T>
const T *LStorage<T>::GetPtr(const std::string &uniqueID) const
{
    auto target = directory.find(uniqueID);
    if (target == directory.end())
        return NULL;

    return &(target->second);
}

template <typename T>
bool LStorage<T>::Register(const std::string &uniqueID)
{
    auto target = directory.find(uniqueID);
    if (target != directory.end())
        return false;

    directory[uniqueID] = new decltype(*T);
    return true;
}