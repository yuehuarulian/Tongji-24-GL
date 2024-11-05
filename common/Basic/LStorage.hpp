#ifndef _LSTORAGE_H_
#define _LSTORAGE_H_

#include <map>
#include <string>

namespace Basic
{
    // 实现本地存储 LocalStorage
    template <typename T>
    class LStorage
    {
    public:
        // 注册对象
        // 如果对象的 uniqueID 尚未注册，它会将对象 item 存储在 directory 中并返回 true；
        // 如果对象已存在，则替换已有对象并返回 false。
        bool Register(const std::string &uniqueID, const T &item);

        // 注销对象，根据 uniqueID 查找并移除对应的对象。如果找不到，则返回 false。
        bool Unregister(const std::string &uniqueID);

        // 获取存储对象的指针，根据 uniqueID 查找并返回指向对象的指针。
        T *GetPtr(const std::string &uniqueID);

        const T *GetPtr(const std::string &uniqueID) const;

        // 仅注册一个空对象（或默认对象）
        bool Register(const std::string &uniqueID);

    private:
        std::map<std::string, T> directory;
    };
}

#endif