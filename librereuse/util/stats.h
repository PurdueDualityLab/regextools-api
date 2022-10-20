//
// Created by charlie on 11/17/21.
//

#ifndef _STATS_H
#define _STATS_H

#include <numeric>
#include <iterator>
#include <type_traits>
#include <valarray>

#include "is_iterable.h"

namespace rereuse::util {
    template<typename Iterator>
    double mean(Iterator begin, Iterator end) {
        auto size = std::distance(begin, end);
        static_assert(std::is_arithmetic<typename std::iterator_traits<Iterator>::value_type>::value);
        auto sum = (double) std::reduce(begin, end);
        return sum / (double) size;
    }

    template<typename Iterator>
    double stddev(Iterator begin, Iterator end) {
        static_assert(std::is_arithmetic<typename std::iterator_traits<Iterator>::value_type>::value);

        auto size = std::distance(begin, end);

        double mean_val = mean(begin, end);

        double sq_sum = std::inner_product(begin, end, begin, 0.0);
        auto dsize = (double) size;
        return std::sqrt(sq_sum / dsize - mean_val * mean_val);
    }

    template <typename Iterator>
    double median(Iterator begin, Iterator end) {
        static_assert(std::is_arithmetic<typename std::iterator_traits<Iterator>::value_type>::value);
        using value_type = typename std::iterator_traits<Iterator>::value_type;

        // Make a vector out of the data structure, sorted
        std::vector<value_type> values(begin, end);
        std::sort(values.begin(),  values.end());

        // Return the middle value
        double median;
        if (values.size() % 2 == 0) {
            // Even number
            int left_idx = (values.size() / 2) - 1;
            int right_idx = (values.size() / 2);
            median = ((double) (values[left_idx] + values[right_idx])) / ((double) 2);
        } else {
            int middle = values.size() / 2;
            median = values[middle];
        }

        return median;
    }

    template <typename Iterator>
    std::pair<typename std::iterator_traits<Iterator>::value_type, typename std::iterator_traits<Iterator>::value_type>
    quartiles(Iterator begin, Iterator end) {
        static_assert(std::is_arithmetic<typename std::iterator_traits<Iterator>::value_type>::value);
        using value_type = typename std::iterator_traits<Iterator>::value_type;

        // Make a vector out of the data structure, sorted
        std::vector<value_type> values(begin, end);
        std::sort(values.begin(),  values.end());

        // get the first quartile
        int first_idx = (values.size()) / 4;
        auto first = values[first_idx];

        int third_idx = (values.size()) * 3 / 4;
        auto third = values[third_idx];

        return std::make_pair(first, third);
    }

    template <typename Iterator>
    std::vector<typename std::iterator_traits<Iterator>::value_type>
    outliers(Iterator begin, Iterator end) {
        // Make sure the iterators are not constant
        // static_assert(!std::is_const<typename std::iterator_traits<Iterator>::value_type>::value);
        static_assert(std::is_arithmetic<typename std::iterator_traits<Iterator>::value_type>::value);
        using value_type = typename std::iterator_traits<Iterator>::value_type;

        auto dataset_quartiles = quartiles(begin, end);
        auto iqr = dataset_quartiles.second - dataset_quartiles.first;
        double lower_fence = dataset_quartiles.first - 1.5 * iqr;
        double upper_fence = dataset_quartiles.second + 1.5 * iqr;

        // Get the outliers
        std::vector<value_type> outliers;
        for (auto it = begin; it != end; ++it) {
            auto val = *it;
            if (val < lower_fence || val > upper_fence) {
                outliers.push_back(val);
            }
        }

        return outliers;
    }

    template <typename Container>
    void remove_outliers(Container &container) {
        // Make sure the container is an iterable
        static_assert(rereuse::util::is_iterable_v<Container>);

        auto outs = outliers(std::begin(container), std::end(container));

        for (auto it = std::begin(container); it != std::end(container);) {
            if (std::count(outs.cbegin(),  outs.cend(), *it)) {
                it = container.erase(it);
            } else {
                it++;
            }
        }
    }

    template <typename DurationIter>
    auto mean_duration(DurationIter begin, DurationIter end) -> typename std::iterator_traits<DurationIter>::value_type {
        using ValueType = typename std::iterator_traits<DurationIter>::value_type;
        static_assert(std::is_same_v<
                std::chrono::duration<
                        typename ValueType::rep,
                        typename ValueType::period
                >,
                ValueType>);

        auto container_size = std::distance(begin, end);
        if (container_size == 0)
            return ValueType();

        ValueType sum;
        for (auto it = begin; it != end; ++it) {
            sum += *it;
        }

        return sum / container_size;
    }

    template <typename DurationIter>
    auto median_duration(DurationIter begin, DurationIter end) -> typename std::iterator_traits<DurationIter>::value_type {
        using ValueType = typename std::iterator_traits<DurationIter>::value_type;
        static_assert(std::is_same_v<
                std::chrono::duration<
                        typename ValueType::rep,
                        typename ValueType::period
                        >,
                ValueType>);

        if (std::distance(begin, end) == 0)
            return ValueType();

        std::vector<typename std::iterator_traits<DurationIter>::value_type> copied_durations(begin, end);
        std::sort(copied_durations.begin(), copied_durations.end());

        ValueType median;
        auto container_size = copied_durations.size();
        if (container_size % 2 == 0) {
            // Even number
            int left_idx = (container_size / 2) - 1;
            int right_idx = (container_size / 2);
            median = copied_durations[left_idx] + copied_durations[right_idx] / 2;
        } else {
            int middle = container_size / 2;
            median = copied_durations[middle];
        }

        return median;
    }
}

#endif //_STATS_H
