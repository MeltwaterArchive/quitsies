package(default_visibility = ["//visibility:public"])

cc_library(
    name = "stats",
    copts = [
        "-I./src",
    ],
    srcs = [
        "statsd_aggregator.cpp",
        "statsd.cpp",
    ],
    hdrs = [
        "aggregator.hpp",
        "null_aggregator.hpp",
        "statsd_aggregator.hpp",
        "statsd.hpp",
    ],
)
