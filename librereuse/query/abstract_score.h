//
// Created by charlie on 8/30/21.
//

#ifndef _ABSTRACTSCORE_H
#define _ABSTRACTSCORE_H

namespace rereuse::query {
    class AbstractScore {
    public:
        virtual ~AbstractScore() = default;
        /**
         * Calculates a score for a regex based on how well it matched against criteria
         * @param tp True positive, matched when supposed to
         * @param tn True negatives, rejected when supposed to
         * @param fp False positives, matched when shouldn't
         * @param fn False negatives, rejected when shouldn't
         * @return some score
         */
        virtual double score(int tp, int tn, int fp, int fn) = 0;

        /**
         * Determines if a score is good enough to pass
         * @param score Score calculated for some regex
         * @return If the score passes whatever criteria is given
         */
        virtual bool score_passes(double score) = 0;
    };
}

#endif //_ABSTRACTSCORE_H
