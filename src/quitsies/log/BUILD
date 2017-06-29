package(default_visibility = ["//visibility:public"])

cc_library(
    name = "log",
    copts = [
        "-I./src",
    ],
    srcs = [
        "logger.cpp",
    ],
    hdrs = [
        "logger.hpp",
    ],
    deps = [
        "//src/spdlog:spdlog",
    ],
)
