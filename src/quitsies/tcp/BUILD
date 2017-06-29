package(default_visibility = ["//visibility:public"])

cc_library(
    name = "tcp",
    copts = [
        "-I./src",
    ],
    srcs = [
        "connection.cpp",
        "connection_manager.cpp",
        "request.cpp",
        "server.cpp",
    ],
    hdrs = [
        "connection.hpp",
        "connection_manager.hpp",
        "request.hpp",
        "server.hpp",
    ],
    deps = [
        "//src/quitsies/db:db",
        "//src/quitsies/log:log",
        "//src/quitsies/stats:stats",
        "@boost//:asio",
        "//external:served",
    ],
)

cc_test(
    name = "tcp_test",
    timeout = "short",
    copts = [
        "-I./src",
    ],
    srcs = [
        "request.test.cpp",
    ],
    deps = [
        ":tcp",
        "//src/quitsies/db:db",
        "//src/quitsies/log:log",
        "//src/test:test",
        "//src/quitsies/stats:stats",
        "@boost//:asio",
        "//external:served",
    ],
)
