# Need that archive
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# Set up go
http_archive(
    name = "io_bazel_rules_go",
    sha256 = "16e9fca53ed6bd4ff4ad76facc9b7b651a89db1689a2877d6fd7b82aa824e366",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_go/releases/download/v0.34.0/rules_go-v0.34.0.zip",
        "https://github.com/bazelbuild/rules_go/releases/download/v0.34.0/rules_go-v0.34.0.zip",
    ],
)

http_archive(
    name = "bazel_gazelle",
    sha256 = "501deb3d5695ab658e82f6f6f549ba681ea3ca2a5fb7911154b5aa45596183fa",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-gazelle/releases/download/v0.26.0/bazel-gazelle-v0.26.0.tar.gz",
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.26.0/bazel-gazelle-v0.26.0.tar.gz",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")
load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")
load("//:deps.bzl", "go_dependencies")

# gazelle:repository_macro deps.bzl%go_dependencies
go_dependencies()

go_rules_dependencies()

# go_register_toolchains(version = "1.19")
# go_register_toolchains(version = "host")

# gazelle_dependencies()
gazelle_dependencies(go_sdk = "go_sdk")

# Set up C++ dependencies
http_archive(
    name = "rules_cc",
    sha256 = "35f2fb4ea0b3e61ad64a369de284e4fbbdcdba71836a5555abb5e194cf119509",
    strip_prefix = "rules_cc-624b5d59dfb45672d4239422fa1e3de1822ee110",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
        "https://github.com/bazelbuild/rules_cc/archive/624b5d59dfb45672d4239422fa1e3de1822ee110.tar.gz",
    ],
)

# RE2 - regex engine dependency
# This version is the original Google version
# http_archive(
#     name = "com_googlesource_code_re2",
#     sha256 = "f89c61410a072e5cbcf8c27e3a778da7d6fd2f2b5b1445cd4f4508bee946ab0f",
#     strip_prefix = "re2-2022-06-01",
#     url = "https://github.com/google/re2/archive/refs/tags/2022-06-01.tar.gz",
# )
# Use this if you have the duality lab version cloned locally
# local_repository(
#     name = "com_googlesource_code_re2",
#     path = "/home/charlie/Programming/re2",
# )
git_repository(
    name = "com_googlesource_code_re2",
    commit = "2546b2bee1354e083f2bfc3143c0e60de178d20f",
    remote = "git@github.com:PurdueDualityLab/re2.git",
    shallow_since = "1668480202 -0500",
)

# github.com/nlohmann/json - C++ JSON parsing utilities
http_archive(
    name = "com_github_nlohmann_json",
    build_file = "@//:third_party/nlohmann_json.BUILD",
    sha256 = "d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273",
    strip_prefix = "json-3.11.2",
    url = "https://github.com/nlohmann/json/archive/refs/tags/v3.11.2.tar.gz",
)

# work-stealing-queue -- A tool that enables threads to steal work from one another
new_git_repository(
    name = "com_github_taskflow_wsq",
    build_file = "@//:third_party/wsq.BUILD",
    commit = "378e297749374300bf9bc0229096285447993877",
    remote = "https://github.com/taskflow/work-stealing-queue.git",
    shallow_since = "1658336407 -0600",
)

# threadpool - a generic C++11 thread pool
new_git_repository(
    name = "com_github_progschj_threadpool",
    build_file = "@//:third_party/threadpool.BUILD",
    commit = "9a42ec1329f259a5f4881a291db1dcb8f2ad9040",
    remote = "https://github.com/progschj/ThreadPool.git",
    shallow_since = "1411727561 +0200",
)

# fmtlib - used for formatting strings
http_archive(
    name = "com_github_fmtlib_fmt",
    build_file = "@//:third_party/fmtlib.BUILD",
    sha256 = "cceb4cb9366e18a5742128cb3524ce5f50e88b476f1e54737a47ffdf4df4c996",
    strip_prefix = "fmt-9.1.0",
    url = "https://github.com/fmtlib/fmt/releases/download/9.1.0/fmt-9.1.0.zip",
)

# Logging!!
http_archive(
    name = "com_gabime_spdlog",
    build_file = "@//:third_party/spdlog.BUILD",
    sha256 = "697f91700237dbae2326b90469be32b876b2b44888302afbc7aceb68bcfe8224",
    strip_prefix = "spdlog-1.10.0",
    url = "https://github.com/gabime/spdlog/archive/refs/tags/v1.10.0.tar.gz",
)

# egret - regex string generation, specifically our fork of the repo
# git_repository(
#     name = "com_github_dualitylab_egret",
#     remote = "git@github.com:PurdueDualityLab/egret.git",
#     tag = "v0.2.1",
# )
local_repository(
    name = "com_github_dualitylab_egret",
    path = "../egret",
)

# Google test
http_archive(
    name = "com_google_googletest",
    sha256 = "81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2",
    strip_prefix = "googletest-release-1.12.1",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz"],
)

# pugixml - a tiny xml parser used for ONE thing: parsing malloc_info (booo poor library design)
new_git_repository(
    name = "com_github_zeux_pugixml",
    build_file = "@//:third_party/pugixml.BUILD",
    commit = "1dc3266fffdb1f37961172c5c96af2f7e6132789",
    remote = "https://github.com/zeux/pugixml.git",
    shallow_since = "1667954974 -0800",
)

###
# Set up python
###

# Adds python language support
http_archive(
    name = "rules_python",
    sha256 = "a868059c8c6dd6ad45a205cca04084c652cfe1852e6df2d5aca036f6e5438380",
    strip_prefix = "rules_python-0.14.0",
    url = "https://github.com/bazelbuild/rules_python/archive/refs/tags/0.14.0.tar.gz",
)

# Install python dependencies with pip
load("@rules_python//python:pip.bzl", "pip_parse")

pip_parse(
    name = "pip_deps",
    requirements_lock = "//:requirements_lock.txt",
    # quiet = False,
)

load("@pip_deps//:requirements.bzl", pip_install_deps = "install_deps")

pip_install_deps()

###
# Set up protcol buffers and gRPC
http_archive(
    name = "rules_proto_grpc",
    sha256 = "bbe4db93499f5c9414926e46f9e35016999a4e9f6e3522482d3760dc61011070",
    strip_prefix = "rules_proto_grpc-4.2.0",
    urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/4.2.0.tar.gz"],
)

load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_repos", "rules_proto_grpc_toolchains")

rules_proto_grpc_toolchains()

rules_proto_grpc_repos()

load("@rules_proto//proto:repositories.bzl", "rules_proto_dependencies", "rules_proto_toolchains")

rules_proto_dependencies()

rules_proto_toolchains()

load("@rules_proto_grpc//cpp:repositories.bzl", rules_proto_grpc_cpp_repos = "cpp_repos")

rules_proto_grpc_cpp_repos()

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()

load("@rules_proto_grpc//:repositories.bzl", "bazel_gazelle", "io_bazel_rules_go")

bazel_gazelle()

load("@rules_proto_grpc//go:repositories.bzl", rules_proto_grpc_go_repos = "go_repos")

rules_proto_grpc_go_repos()

# Set up docker
http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "b1e80761a8a8243d03ebca8845e9cc1ba6c82ce7c5179ce2b295cd36f7e394bf",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.25.0/rules_docker-v0.25.0.tar.gz"],
)

load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)

container_repositories()

load("@io_bazel_rules_docker//repositories:deps.bzl", container_deps = "deps")

container_deps()

load(
    "@io_bazel_rules_docker//cc:image.bzl",
    _cc_image_repos = "repositories",
)

_cc_image_repos()

load(
    "@io_bazel_rules_docker//go:image.bzl",
    _go_image_repos = "repositories",
)

_go_image_repos()

load(
    "@io_bazel_rules_docker//container:container.bzl",
    "container_pull",
)

container_pull(
    name = "ubuntu_linux_amd64",
    digest = "sha256:2d7ecc9c5e08953d586a6e50c29b91479a48f69ac1ba1f9dc0420d18a728dfc5",
    registry = "index.docker.io",
    repository = "library/ubuntu",
    tag = "22.04",
)
