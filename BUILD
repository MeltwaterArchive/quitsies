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
        "//third_party/boost:system",
        "//third_party/boost:filesystem",
        "//third_party/boost:asio",
        "//external:re2",
        "//external:served",
        "//src/quitsies:options",
        "//src/quitsies/log:log",
        "//src/quitsies/db:db",
        "//src/quitsies/stats:stats",
        "//src/quitsies/tcp:tcp",
    ],
)
