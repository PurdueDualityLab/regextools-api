load("@io_bazel_rules_docker//container:container.bzl", "container_image", "container_push")
load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")

cc_binary(
    name = "regexdb",
    srcs = [
        "main.cc",
        "query_service.cc",
        "query_service.h",
    ],
    deps = [
        "//librereuse:rereuse",
        "//proto:cpp_query_service_proto",
        "@com_gabime_spdlog//:spdlog",
    ],
)

cc_image(
    name = "image",
    base = "@ubuntu_linux_amd64//image",
    binary = ":regexdb",
)

# container_push(
#     name = "image-push",
#     format = "Docker",
#     image = ":image",
#     registry = "registry.hub.docker.com",
#     repository = "softwaresale/regextools-regexdb",
# )
