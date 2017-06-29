package(default_visibility = ["//visibility:public"])

cc_library(
    name = "options",
    copts = [
        "-I./src",
    ],
    srcs = [
        "options.cpp",
    ],
    hdrs = [
        "options.hpp",
    ],
    deps = [
        "//src/OptionHandler:optionhandler",
    ],
)

cc_test(
    name = "options_test",
    timeout = "short",
    copts = [
        "-I./src",
    ],
    srcs = [
        "options.test.cpp",
    ],
    deps = [
        "//src/test:test",
        "//src/OptionHandler:optionhandler",
        ":options",
    ],
)
