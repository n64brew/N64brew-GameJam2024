#ifndef __LIST_H
#define __LIST_H

#include <array>

template<typename T, std::size_t S>
class List : public std::array<T, S> {
    private:
        std::size_t count = 0;
    public:

        void add(T &item) {
            if (count >= this->size()) return;
            (*this)[count++] = item;
        }

        // TODO: return success/failure
        void add(T &&item) {
            if (count >= this->size()) return;
            (*this)[count++] = item;
        }

        void remove(typename std::array<T,S>::iterator &it) {
            if (count == 0) return;
            *it = (*this)[--count];
            // Honestly, I couldn't find a better api for this
            // Actually the iterator is invalid once you remove it
            // and it should behave accordingly
            it--;
        }

        void clear() {
            count = 0;
        }

        auto end() noexcept {
            return this->begin() + count;
        }
};

#endif // __LIST_H
