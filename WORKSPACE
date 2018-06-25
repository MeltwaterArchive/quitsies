workspace(name = "com_github_datasift_quitsies")

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "72ec09168e5c3a296f667b3d956a853ccd65c8ed",
    remote = "https://github.com/nelhage/rules_boost",
)
load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

new_http_archive(
    name = "snappy_archive",
    url = "https://github.com/google/snappy/archive/ed3b7b2.tar.gz",
    strip_prefix = "snappy-ed3b7b242bd24de2ca6750c73f64bee5b7505944",
    sha256 = "88a644b224f54edcd57d01074c2d6fd6858888e915c21344b8622c133c35a337",
    build_file = "//:third_party/snappy.BUILD",
)
bind(
    name = "snappy",
    actual = "@snappy_archive//:snappy",
)
bind(
    name = "snappy_config",
    actual = "//third_party/snappy:config"
)

http_archive(
    name = "com_github_gflags_gflags",  # match the name defined in its WORKSPACE file
    url = "https://github.com/gflags/gflags/archive/9314597.tar.gz",
    strip_prefix = "gflags-9314597d4b742ed6f95665241345e590a0f5759b",
    sha256 = "75155b41074c09b2788e2415c1b6151b663afca9825c1345714a9476438a5336",
)
bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

new_http_archive(
    name = "glog_archive",
    url = "https://github.com/google/glog/archive/da816ea.tar.gz",
    strip_prefix = "glog-da816ea70645e463aa04f9564544939fa327d5a7",
    sha256 = "54fa0b1de92795c877d3fae363a1a014de5c16b7232a159186ee9a1894cd9733",
    build_file = "//:third_party/glog.BUILD",
)
bind(
    name = "glog",
    actual = "@glog_archive//:glog",
)
bind(
    name = "glog_config",
    actual = "//third_party/glog:config",
)

new_git_repository(
    name = "libunwind_git",
    remote = "git://git.sv.gnu.org/libunwind.git",
    tag = "v1.1", # Note: update the version in BUILD file
    build_file = "//:third_party/libunwind.BUILD",
)
bind(
    name = "libunwind",
    actual = "@libunwind_git//:libunwind",
)
bind(
    name = "libunwind_config",
    actual = "//third_party/libunwind:config",
)

new_http_archive(
    name = "gtest_archive",
    url = "https://github.com/google/googletest/archive/release-1.8.0.tar.gz",
    strip_prefix = "googletest-release-1.8.0",
    sha256 = "58a6f4277ca2bc8565222b3bbd58a177609e9c488e8a72649359ba51450db7d8",
    build_file = "//:third_party/gtest.BUILD",
)
bind(
    name = "gtest",
    actual = "@gtest_archive//:gtest",
)

new_http_archive(
    name = "jemalloc_archive",
    url = "https://github.com/jemalloc/jemalloc/archive/4.5.0.tar.gz",
    strip_prefix = "jemalloc-4.5.0",
    sha256 = "e885b65b95426945655ee91a30f563c9679770c92946bcdd0795f6b78c06c221",
    build_file = "//:third_party/jemalloc.BUILD",
)
bind(
    name = "jemalloc",
    actual = "@jemalloc_archive//:jemalloc",
)
bind(
    name = "jemalloc_config",
    actual = "//third_party/jemalloc:config"
)

new_http_archive(
    name = "zlib_archive",
    url = "https://github.com/madler/zlib/archive/v1.2.11.tar.gz",
    strip_prefix = "zlib-1.2.11",
    sha256 = "629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff",
    build_file = "//:third_party/zlib.BUILD",
)
bind(
    name = "zlib",
    actual = "@zlib_archive//:zlib",
)

new_git_repository(
    name = "rocksdb_git",
    remote = "https://github.com/facebook/rocksdb.git",
    tag = "v5.7.3",
    build_file = "//:third_party/rocksdb.BUILD",
)
bind(
    name = "rocksdb",
    actual = "@rocksdb_git//:rocksdb",
)

#git_repository(
    #name = "com_github_datasift_served",
    #remote = "https://github.com/datasift/served",
    #commit = "59d3369c25c910f69dda295d5c009b24ec929236",
    #init_submodules = 1,
#)
http_archive(
    name = "com_github_datasift_served",
    url = "https://github.com/datasift/served/archive/59d3369c25c910f69dda295d5c009b24ec929236.tar.gz",
    strip_prefix = "served-59d3369c25c910f69dda295d5c009b24ec929236",
)
bind(
    name = "served",
    actual = "@com_github_datasift_served//:served",
)
