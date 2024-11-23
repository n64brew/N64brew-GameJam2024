#ifndef __LIST_H
#define __LIST_H

#include <array>

template<typename T, std::size_t S>
class List : public std::array<T, S> {
    private:
        std::size_t count = 0;
    public:
        void add(const T &item) {
            if (count >= this->size()) return;
            auto &newItem = std::array<T,S>::operator[](count);
            newItem = item;
            count++;
        }

        void remove(std::size_t index) {
            if (count == 0) return;
            this->at(index) = std::array<T,S>::operator[](count);
            count--;
        }

        void remove(typename std::array<T,S>::iterator it) {
            *it = std::array<T,S>::operator[](count);
            count--;
        }

        typename std::array<T,S>::iterator end() noexcept {
            return this->begin() + count;
        }
};

#endif // __LIST_H
