#ifndef _PTR_H_
#define _PTR_H_

#include <memory>

/*
    使用方式
    // 假设有一个类 Example
    class Example {};

    // 使用共享指针类型
    CppUtil::Basic::PointerTypes<Example>::Ptr examplePtr = std::make_shared<Example>(); // 可修改
    CppUtil::Basic::PointerTypes<Example>::CPtr constExamplePtr = examplePtr; // 只读
*/

namespace Basic
{
    template <typename T>
    using Ptr = std::shared_ptr<T>;

    template <typename T>
    using CPtr = std::shared_ptr<const T>;
}

#endif // !_PTR_H_