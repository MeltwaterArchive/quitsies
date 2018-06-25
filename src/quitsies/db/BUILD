package(default_visibility = ["//visibility:public"])

cc_library(
    name = "db",
    copts = [
        "-I./src",
    ],
    srcs = [
        "rocks.cpp",
    ],
    hdrs = [
        "store.hpp",
        "rocks.hpp",
    ],
    deps = [
        "//src/OptionHandler:optionhandler",
        "//src/quitsies:options",
        "//src/quitsies/log:log",
        "//src/quitsies/stats:stats",
        "@boost//:filesystem",
        "@boost//:asio",
        "//external:rocksdb",
        "//external:served",
    ],
)
