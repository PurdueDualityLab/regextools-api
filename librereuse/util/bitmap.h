//
// Created by charlie on 10/13/22.
//

#ifndef REGEXTOOLS_BITMAP_H
#define REGEXTOOLS_BITMAP_H

#include <vector>
#include <bitset>

namespace rereuse::util {

    /**
     * Represents a dynamically sized bitmap
     */
    class Bitmap {
    public:
        static constexpr unsigned long BITSET_SIZE = 8;
        using Bitset = std::bitset<Bitmap::BITSET_SIZE>;

        explicit Bitmap(unsigned long elements);
        explicit Bitmap(unsigned long elements, bool one_or_zero_default);

        std::size_t bitset_count() const { return this->bitset.size(); }
        unsigned long size() const { return this->element_count; }

        bool get(std::size_t element_idx) const;

        void set(std::size_t element_idx, bool value);
        void toggle(std::size_t element_idx);

        Bitmap operator&(const Bitmap &bitmap) const;
        Bitmap &operator&=(const Bitmap &bitmap);
        Bitmap operator|(const Bitmap &bitmap) const;
        Bitmap &operator|=(const Bitmap &bitmap);
        Bitmap operator^(const Bitmap &bitmap) const;
        Bitmap &operator^=(const Bitmap &bitmap);
        Bitmap operator~() const;

    private:
        std::pair<std::size_t, unsigned char> lookup_element_idx(std::size_t element_idx) const;

        unsigned long element_count;
        std::vector<Bitset> bitset;
    };
} // util

#endif //REGEXTOOLS_BITMAP_H
