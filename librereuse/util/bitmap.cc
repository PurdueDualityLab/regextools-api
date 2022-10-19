
// Created by charlie on 10/13/22.
//

#include "bitmap.h"

#include <cmath>

rereuse::util::Bitmap::Bitmap(unsigned long elements)
: element_count(elements) {
    auto vector_size = std::ceil(elements / static_cast<double>(BITSET_SIZE));
    this->bitset = std::vector<Bitset>(static_cast<unsigned long>(vector_size), Bitmap::Bitset(0));
}

rereuse::util::Bitmap::Bitmap(unsigned long elements, bool one_or_zero_default)
: element_count(elements) {
    auto vector_size = std::ceil(elements / static_cast<double>(BITSET_SIZE));
    this->bitset = std::vector<Bitset>(static_cast<unsigned long>(vector_size), Bitmap::Bitset(one_or_zero_default ? std::numeric_limits<unsigned char>::max() : 0));
}

bool rereuse::util::Bitmap::get(std::size_t element_idx) const {
    auto [bitset_idx, bit_idx] = this->lookup_element_idx(element_idx);
    return this->bitset[bitset_idx][bit_idx];
}

void rereuse::util::Bitmap::set(std::size_t element_idx, bool value) {
    auto [bitset_idx, bit_idx] = this->lookup_element_idx(element_idx);
    this->bitset[bitset_idx][bit_idx] = value;
}

void rereuse::util::Bitmap::toggle(std::size_t element_idx) {
    auto [bitset_idx, bit_idx] = this->lookup_element_idx(element_idx);
    this->bitset[bitset_idx][bit_idx].flip();
}

std::pair<std::size_t, unsigned char> rereuse::util::Bitmap::lookup_element_idx(std::size_t element_idx) const {
    auto bitsets_idx = static_cast<std::size_t>(element_idx / static_cast<double>(BITSET_SIZE));
    auto bit_idx = static_cast<unsigned char>(element_idx % BITSET_SIZE);

    return { bitsets_idx, bit_idx };
}

rereuse::util::Bitmap rereuse::util::Bitmap::operator&(const rereuse::util::Bitmap &bitmap) const {
    Bitmap new_bm(std::min(bitmap.size(), this->size()));
    for (unsigned long idx = 0; idx < new_bm.bitset_count(); idx++) {
        new_bm.bitset[idx] = this->bitset[idx] & bitmap.bitset[idx];
    }

    return new_bm;
}

rereuse::util::Bitmap &rereuse::util::Bitmap::operator&=(const rereuse::util::Bitmap &bitmap) {
    auto bitset_count = this->bitset_count();
    for (unsigned long i = 0; i < bitset_count; i++) {
        this->bitset[i] &= bitmap.bitset[i];
    }

    return *this;
}

rereuse::util::Bitmap rereuse::util::Bitmap::operator|(const rereuse::util::Bitmap &bitmap) const {
    // Start with the larger of the two
    Bitmap new_bm = (this->element_count > bitmap.element_count)
            ? *this
            : bitmap;

    const Bitmap &to_compare = (this->element_count < bitmap.element_count)
                               ? *this
                               : bitmap;

    for (unsigned long i = 0; i < to_compare.bitset_count(); i++) {
        new_bm.bitset[i] |= to_compare.bitset[i];
    }

    return new_bm;
}

rereuse::util::Bitmap &rereuse::util::Bitmap::operator|=(const rereuse::util::Bitmap &bitmap) {
    // if bitmap is larger than this, then this needs to be expanded
    if (bitmap.size() > this->size()) {
        this->bitset.resize(bitmap.size());
    }

    for (unsigned long idx = 0; idx < this->bitset_count(); idx++) {
        this->bitset[idx] |= bitmap.bitset[idx];
    }

    return *this;
}

rereuse::util::Bitmap rereuse::util::Bitmap::operator~() const {
    Bitmap new_bm = *this;
    for (auto &sub_bitset : new_bm.bitset) {
        sub_bitset.flip();
    }

    return new_bm;
}

rereuse::util::Bitmap rereuse::util::Bitmap::operator^(const rereuse::util::Bitmap &bitmap) const {
    // Start with the larger of the two
    Bitmap new_bm = (this->element_count > bitmap.element_count)
                    ? *this
                    : bitmap;

    const Bitmap &to_compare = (this->element_count < bitmap.element_count)
                               ? *this
                               : bitmap;

    for (unsigned long i = 0; i < to_compare.bitset_count(); i++) {
        new_bm.bitset[i] ^= to_compare.bitset[i];
    }

    return new_bm;
}

rereuse::util::Bitmap &rereuse::util::Bitmap::operator^=(const rereuse::util::Bitmap &bitmap) {
    // if bitmap is larger than this, then this needs to be expanded
    if (bitmap.size() > this->size()) {
        this->bitset.resize(bitmap.size());
    }

    for (unsigned long idx = 0; idx < this->bitset_count(); idx++) {
        this->bitset[idx] ^= bitmap.bitset[idx];
    }

    return *this;
}
