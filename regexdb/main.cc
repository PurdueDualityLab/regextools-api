//
// Created by charlie on 8/25/22.
//

#include <iostream>
#include <memory>
#include <grpc++/grpc++.h>
#include "query_service.h"

int main(int argc, const char **argv) {
    regextools::QueryServiceImpl service(7, "/home/charlie/Programming/redb/db-20000-clusters.json");
    grpc::ServerBuilder serverBuilder;
    serverBuilder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    serverBuilder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(serverBuilder.BuildAndStart());
    std::cout << "Server listening on 0.0.0.0:50051" << std::endl;
    server->Wait();

    return 0;
}
