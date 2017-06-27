licenses(["notice"])

exports_files(["LICENSE"])

cc_binary(
    name = "quitsies",
    srcs = [ "src/service.cpp" ],
    copts = [
        "-Isrc",
    ],
    deps = [
        "//external:rocksdb",
        "//external:served",
        "@boost//:system",
        "@boost//:filesystem",
        "@boost//:asio",
        "//external:re2",
        "//:quitsies-lib",
    ],
)

cc_library(
    name = "quitsies-lib",
    srcs = glob(
        [
            "src/quitsies/*.cpp",
            "src/quitsies/db/*.cpp",
            "src/quitsies/tcp/*.cpp",
            "src/quitsies/stats/*.cpp",
            "src/quitsies/log/*.cpp",
        ],
        exclude = [
            "src/quitsies/*.test.cpp",
            "src/quitsies/db/*.test.cpp",
            "src/quitsies/tcp/*.test.cpp",
            "src/quitsies/stats/*.test.cpp",
            "src/quitsies/log/*.test.cpp",
        ],
    ),
    hdrs = glob(
        [
            "src/quitsies/*.hpp",
            "src/quitsies/db/*.hpp",
            "src/quitsies/tcp/*.hpp",
            "src/quitsies/stats/*.hpp",
            "src/quitsies/log/*.hpp",
            "src/OptionHandler/option_handler.h",
            "src/spdlog/*.h",
            "src/spdlog/details/*.h",
            "src/spdlog/fmt/*.h",
            "src/spdlog/fmt/bundled/*.cc",
            "src/spdlog/fmt/bundled/*.h",
            "src/spdlog/sinks/*.h",
        ],
    ),
    strip_include_prefix = "src/",
    visibility = ["//visibility:public"],
    deps = [
        "//external:rocksdb",
        "//external:served",
        "@boost//:system",
        "@boost//:filesystem",
        "@boost//:asio",
        "//external:re2",
    ],
)

cc_test(
    name = "quitsies-test",
    copts = [
        "-Isrc",
    ],
    tags = [ "large" ],
    timeout = "short",
    srcs = glob(
        [
            "src/quitsies/*.test.cpp",
            "src/quitsies/**/*.test.cpp",
            "src/test/catch.cpp",
            "src/test/catch.hpp",
        ],
    ),
    deps = [
        "//external:rocksdb",
        "//external:served",
        "@boost//:system",
        "@boost//:filesystem",
        "@boost//:asio",
        "//external:re2",
        "//:quitsies-lib",
    ],
)
