//
// Created by charlie on 10/12/22.
//

#include <gtest/gtest.h>
#include "librereuse/query/cluster_match_query.h"
#include "librereuse/db/regex_cluster_repository.h"

#include <string>
#include <unordered_set>

class ClusterMatchQueryFixture : public ::testing::Test {
protected:
    const std::unordered_set<std::string> all_patterns = {
            "[a-z]+",
            "[a-zA-Z]+",
            "(abc)+",
            "((abc)|(ABC))+",
            "(def)+",
            "((def)|(DEF))+",
            "[^def]+",
    };

    const std::vector<std::unordered_set<std::string>> cluster_patterns = {
            {
                    "[a-zA-Z]+",
                    "(abc)+",
                    "((abc)|(ABC))+",},
            {
                    "[a-z]+",
                    "(def)+",
                    "((def)|(DEF))+",}
    };

    std::shared_ptr<rereuse::db::Cluster> big_cluster;

    void SetUp() override {
        Test::SetUp();

        big_cluster = std::make_shared<rereuse::db::Cluster>(all_patterns);
        big_cluster->compile();
    }
};

TEST_F(ClusterMatchQueryFixture, should_get_all_matches) {
    rereuse::query::ClusterMatchQuery query({ "abc" }, { "def" });
    auto is_candidate = query.test(this->big_cluster, nullptr);
    EXPECT_TRUE(is_candidate);

    auto results = query.query(this->big_cluster, nullptr);
    EXPECT_EQ(results.size(), 3);
}

TEST_F(ClusterMatchQueryFixture, reject_on_positives) {
    rereuse::query::ClusterMatchQuery query({ "cmsale@purdue.edu" }, { "def" });
    auto is_candidate = query.test(this->big_cluster, nullptr);
    EXPECT_FALSE(is_candidate);

    auto results = query.query(this->big_cluster, nullptr);
    EXPECT_EQ(results.size(), 0);
}

TEST_F(ClusterMatchQueryFixture, reject_on_negative) {
    auto smaller_cluster = std::make_shared<rereuse::db::Cluster>(cluster_patterns[0]);
    smaller_cluster->compile();

    rereuse::query::ClusterMatchQuery query({}, { "abc" });
    auto is_candidate = query.test(smaller_cluster, nullptr);
    EXPECT_FALSE(is_candidate);

    auto results = query.query(smaller_cluster, nullptr);
    EXPECT_EQ(results.size(), 0);
}
