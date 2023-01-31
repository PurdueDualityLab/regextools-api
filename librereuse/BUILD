cc_library(
    name = "rereuse",
    srcs = [
        "//librereuse/db:librereuse_db_hdrs",
        "//librereuse/db:librereuse_db_srcs",
        "//librereuse/query:librereuse_query_hdrs",
        "//librereuse/query:librereuse_query_srcs",
        "//librereuse/util:librereuse_util_hdrs",
        "//librereuse/util:librereuse_util_srcs",
    ],
    hdrs = [
        "//librereuse/db:librereuse_db_hdrs",
        "//librereuse/query:librereuse_query_hdrs",
        "//librereuse/util:librereuse_util_hdrs",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@com_gabime_spdlog//:spdlog",
        "@com_github_dualitylab_egret//src:egret-lib",
        "@com_github_nlohmann_json//:json",
        "@com_github_softwaresale_threadpool//:threadpool",
        "@com_github_taskflow_wsq//:work_stealing_queue",
        "@com_googlesource_code_re2//:re2",
    ],
)
