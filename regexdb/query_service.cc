//
// Created by charlie on 8/25/22.
//

#include "query_service.h"

#include "spdlog/spdlog.h"
#include "librereuse/db/pattern_reader.h"

static ::regextools::RegexEntity grpc_entity_from_rereuse_entity(const rereuse::db::RegexEntity &entity) {
    ::regextools::RegexEntity new_entity;
    new_entity.set_pattern(entity.get_pattern());
    new_entity.set_id(entity.get_id());

    return new_entity;
}

regextools::QueryServiceImpl::QueryServiceImpl(unsigned int workers, const std::string &path) {
    auto clusters = rereuse::db::read_semantic_clusters(path);
    repo = std::make_unique<rereuse::db::ParallelRegexClusterRepository>(workers);
    spdlog::debug("QueryService: read {} semantic clusters", clusters.size());
    unsigned long cluster_count = 0;
    for (auto &cluster : clusters) {
        if (repo->add_cluster(std::move(cluster))) {
            cluster_count++;
        }
    }

    spdlog::info("QueryService: Loaded {} clusters", cluster_count);
}


grpc::Status regextools::QueryServiceImpl::ExecQuery(::grpc::ServerContext *context, const ::regextools::Query *request,
                                                     ::regextools::QueryResults *response) {

    std::unordered_set<std::string> positive_examples(request->positive().begin(), request->positive().end());
    std::unordered_set<std::string> negative_examples(request->negative().begin(), request->negative().end());
    std::unique_ptr<rereuse::query::BaseClusterQuery> query = std::make_unique<rereuse::query::ClusterMatchQuery>(positive_examples, negative_examples);

    auto results = this->repo->query(query);
    for (const auto &result : results) {
        auto new_result = grpc_entity_from_rereuse_entity(result);
        response->mutable_results()->Add(std::move(new_result));
    }

    response->set_total(results.size());

    spdlog::info("Handled query. Got {} results", results.size());
    return grpc::Status::OK;
}
