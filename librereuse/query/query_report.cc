//
// Created by charlie on 11/29/22.
//

#include "query_report.h"

rereuse::query::QueryResult::QueryResult(std::vector<db::RegexEntity> results, unsigned long skippedClusters,
                                         const std::chrono::microseconds &medianTestFailTime,
                                         const std::chrono::microseconds &medianTestPassTime,
                                         const std::chrono::microseconds &medianDrillTime,
                                         double averageMatchVectorSize)
: results(std::move(results))
, skipped_clusters(skippedClusters)
, median_test_fail_time(medianTestFailTime)
, median_test_pass_time(medianTestPassTime)
, median_drill_time(medianDrillTime)
, average_match_vector_size(averageMatchVectorSize)
{ }

rereuse::query::QueryReport::QueryReport(const rereuse::query::QueryResult &result)
: results(result.results)
, positive_examples_count(0)
, negative_examples_count(0)
, skipped_clusters(result.skipped_clusters)
, total_elapsed_time()
, median_test_pass_time(result.median_test_pass_time)
, median_test_fail_time(result.median_test_fail_time)
, median_drill_time(result.median_drill_time)
, average_vec_size(result.average_match_vector_size)
, result_cluster_info()
{}

rereuse::query::QueryReport::QueryReport(rereuse::query::QueryResult &&result)
: results(std::move(result.results))
, positive_examples_count(0)
, negative_examples_count(0)
, skipped_clusters(result.skipped_clusters)
, total_elapsed_time()
, median_test_pass_time(result.median_test_pass_time)
, median_test_fail_time(result.median_test_fail_time)
, median_drill_time(result.median_drill_time)
, average_vec_size(result.average_match_vector_size)
, result_cluster_info()
{}

rereuse::query::QueryReport::QueryReport()
: positive_examples_count(0)
, negative_examples_count(0)
, skipped_clusters(0)
, total_elapsed_time()
, average_vec_size(0)
, result_cluster_info()
{}