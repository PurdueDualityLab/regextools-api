//
// Created by charlie on 8/30/21.
//

#include "miss_score.h"

rereuse::query::MissScore::MissScore() : max_misses(0) {
}

rereuse::query::MissScore::MissScore(int maxMisses) : max_misses(maxMisses) {
}

double rereuse::query::MissScore::score(int tp, int tn, int fp, int fn) {
    return fp + fn;
}

bool rereuse::query::MissScore::score_passes(double score) {
    return score <= this->max_misses;
}
