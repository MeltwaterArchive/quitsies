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
        "//external:served",
        "@boost//:system",
        "@boost//:filesystem",
        "@boost//:asio",
        "//external:re2",
        "//src/quitsies:options",
        "//src/quitsies/log:log",
        "//src/quitsies/db:db",
        "//src/quitsies/stats:stats",
        "//src/quitsies/tcp:tcp",
    ],
)
