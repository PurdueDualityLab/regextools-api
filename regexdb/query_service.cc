//
// Created by charlie on 8/25/22.
//

#include "query_service.h"

#include "librereuse/db/pattern_reader.h"

regextools::QueryServiceImpl::QueryServiceImpl(unsigned int workers, const std::string &path) {
    auto clusters = rereuse::db::read_semantic_clusters(path);
    repo = std::make_unique<rereuse::db::ParallelRegexClusterRepository>(workers);
    for (auto &cluster : clusters) {
        repo->add_cluster(std::move(cluster));
    }
}


grpc::Status regextools::QueryServiceImpl::ExecQuery(::grpc::ServerContext *context, const ::regextools::Query *request,
                                                     ::regextools::QueryResults *response) {

    std::unordered_set<std::string> positive_examples(request->positive().begin(), request->positive().end());
    std::unordered_set<std::string> negative_examples(request->negative().begin(), request->negative().end());
    std::shared_ptr<rereuse::query::BaseClusterQuery> query = std::make_shared<rereuse::query::ClusterMatchQuery>(positive_examples, negative_examples);

    auto results = this->repo->query(query);
    for (auto &result : results) {
        response->add_results(result);
    }

    response->set_total(results.size());

    std::cout << "Handled instance. Got " << results.size() << " results" << std::endl;
    return grpc::Status::OK;
}
