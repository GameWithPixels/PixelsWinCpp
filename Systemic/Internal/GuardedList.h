#pragma once

#include <iterator>
#include <list>
#include <vector>
#include <mutex>

namespace Systemic
{
    /**
     * @brief A simple thread safe list of items.
     * @tparam T The item type.
     */
    template <typename T>
    class GuardedList
    {
        std::list<T> _list{};
        mutable std::recursive_mutex _mutex{};

    public:
        /// Type of an item index.
        using Index = typename std::list<T>::const_iterator;

        /**
         * @brief Gets a copy of the list of items.
         * @return A copy of the list of items.
         */
        std::vector<T> get() const
        {
            std::lock_guard lock{ _mutex };
            return std::vector<T>{ _list.begin(), _list.end() };
        }

        /**
         * @brief Adds the given item to the list and returns its index.
         * @param item The item to add.
         * @return The index of the added item.
         */
        Index add(const T& item)
        {
            std::lock_guard lock{ _mutex };
            _list.push_back(item);
            return std::prev(_list.cend());
        }

        /**
         * @brief Removes the item at the given index from the list.
         * @param index The index of the item to remove.
         */
        void remove(const Index& index)
        {
            std::lock_guard lock{ _mutex };
            _list.erase(index);
        }
    };
}
