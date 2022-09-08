//
// Created by charlie on 8/25/22.
//

#ifndef REGEXTOOLS_QUERY_SERVICE_H
#define REGEXTOOLS_QUERY_SERVICE_H

#include <proto/query_service.grpc.pb.h>
#include "librereuse/db/parallel_regex_cluster_repository.h"

namespace regextools {
    class QueryServiceImpl final : public QueryService::Service {
    public:
        explicit QueryServiceImpl(unsigned int workers, const std::string &path);

        grpc::Status ExecQuery(::grpc::ServerContext *context, const ::regextools::Query *request,
                               ::regextools::QueryResults *response) override;

    private:
        std::unique_ptr<rereuse::db::RegexClusterRepository> repo;
    };
}

#endif //REGEXTOOLS_QUERY_SERVICE_H
