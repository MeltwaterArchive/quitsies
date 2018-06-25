licenses(["notice"])

exports_files(["LICENSE"])

cc_binary(
    name = "quitsies",
    srcs = [ "src/service.cpp" ],
    copts = [
        "-I./src",
    ],
    deps = [
        "//external:rocksdb",
        "@boost//:system",
        "@boost//:filesystem",
        "@boost//:asio",
        "//external:served",
        "//src/quitsies:options",
        "//src/quitsies/log:log",
        "//src/quitsies/db:db",
        "//src/quitsies/stats:stats",
        "//src/quitsies/tcp:tcp",
    ],
)

load("@io_bazel_rules_docker//cc:image.bzl", "cc_image")

cc_image(
    name = "quitsies-docker",
	binary = ":quitsies",
)
