
# regextools.io - backend services

This repository contains the backend for regextools.io. The backend is responsible for
services like querying regexes and other tasks.

To build:
1. Install Bazel build tool ([instructions](https://bazel.build/install/bazelisk)).
2. In the root of this directory, run `$ bazel sync`
3. Run `$ bazel build //regexdb:image` to build a regex database docker image
4. Run `$ bazel build //server:image` to build the API server docker image
