load("@rules_proto//proto:defs.bzl", "proto_library")
load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load("@rules_proto_grpc//cpp:defs.bzl", "cpp_grpc_library")
load("@rules_proto_grpc//go:defs.bzl", "go_grpc_compile", "go_grpc_library")
load("@rules_proto_grpc//java:defs.bzl", "java_grpc_library")

########################
# Query service protos #
########################
proto_library(
    name = "query_service",
    srcs = ["query_service.proto"],
    visibility = ["//visibility:public"],
)

cpp_grpc_library(
    name = "cpp_query_service_proto",
    protos = [
        ":query_service",
    ],
    visibility = ["//visibility:public"],
)

go_grpc_compile(
    name = "go_query_service_proto",
    protos = [
        ":query_service",
    ],
    visibility = ["//visibility:public"],
)

go_proto_library(
    name = "regextools_go_proto",
    compilers = ["@io_bazel_rules_go//proto:go_grpc"],
    importpath = "github.com/softwaresale/regextools-server/proto",
    proto = ":query_service",
    visibility = ["//visibility:public"],
)

go_library(
    name = "proto",
    embed = [":regextools_go_proto"],
    importpath = "github.com/softwaresale/regextools-server/proto",
    visibility = ["//visibility:public"],
)

go_proto_library(
    name = "query_service_go_proto",
    compilers = ["@io_bazel_rules_go//proto:go_grpc"],
    importpath = "github.com/regextools/protos/query_service",
    proto = ":query_service",
    visibility = ["//visibility:public"],
)

###########################
# Coverage service protos #
###########################
proto_library(
    name = "dfa_coverage_service",
    srcs = ["dfa_cover_service.proto"],
    visibility = ["//visibility:public"],
)

java_grpc_library(
    name = "dfa_coverage_service_java",
    protos = [":dfa_coverage_service"],
    visibility = ["//visibility:public"],
)

go_grpc_library(
    name = "dfa_coverage_service_go",
    importpath = "github.com/regextools/protos/dfa_cover_serivce",
    protos = [":dfa_coverage_service"],
    visibility = ["//visibility:public"],
)
