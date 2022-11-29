load("@bazel_gazelle//:def.bzl", "gazelle")

# gazelle:prefix github.com/softwaresale/regextools-server
# gazelle:go_generate_proto true
gazelle(name = "gazelle")

gazelle(
    name = "gazelle-update-repos",
    args = [
        "-from_file=server/go.mod",
        "-to_macro=deps.bzl%go_dependencies",
        "-prune",
    ],
    command = "update-repos",
)

# load("@rules_python//python:pip.bzl", "compile_pip_requirements")
# compile_pip_requirements(
#   name = "requirements",
#   requirements_in = "//reports:requirements.in",
#   requirements_out = "//:requirements_lock.txt",
# )
