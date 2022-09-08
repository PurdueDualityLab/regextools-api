//
// Created by charlie on 8/25/22.
//

#include <iostream>
#include <memory>
#include <grpc++/grpc++.h>
#include "query_service.h"

int main(int argc, const char **argv) {

    // std::string fallback = "/home/charlie/Programming/redb/db-20000-clusters.json";

    const char *clusterFile = getenv("REGEXDB_CLUSTER_FILE");
    if (!clusterFile) {
        throw std::runtime_error("REGEXDB_CLUSTER_FILE environment variable must be set");
    }

    regextools::QueryServiceImpl service(7, std::string(clusterFile));
    grpc::ServerBuilder serverBuilder;
    serverBuilder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(serverBuilder.BuildAndStart());
    std::cout << "Server listening on 0.0.0.0:50051" << std::endl;
    server->Wait();

    return 0;
}
