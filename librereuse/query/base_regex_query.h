//
// Created by charlie on 8/30/21.
//

#ifndef _BASEREGEXQUERY_H
#define _BASEREGEXQUERY_H

#include <memory>
#include <re2/re2.h>
#include "abstract_score.h"

namespace rereuse::query {
    class BaseRegexQuery {

    public:
        virtual double score(const re2::RE2 &regex) = 0;
        bool test(const re2::RE2 &regex);

    protected:
        explicit BaseRegexQuery(AbstractScore *score);
        explicit BaseRegexQuery(std::shared_ptr<AbstractScore> score);
        virtual ~BaseRegexQuery() = default;
        std::shared_ptr<AbstractScore> scorer;
    };
}

#endif //_BASEREGEXQUERY_H
