//
// Created by charlie on 8/30/21.
//

#ifndef _MISSSCORE_H
#define _MISSSCORE_H

#include "abstract_score.h"

namespace rereuse::query {
    /**
     * Scores a regex based on how well it matched the inputs. It returns the number of misses
     */
    class MissScore : public AbstractScore {
    public:
        explicit MissScore();
        explicit MissScore(int maxMisses);
        double score(int tp, int tn, int fp, int fn) override;

        bool score_passes(double score) override;

    private:
        int max_misses;
    };
}

#endif //_MISSSCORE_H
