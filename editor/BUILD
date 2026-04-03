load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

# -----------------------------------------------------------
# Editor Library
# -----------------------------------------------------------
cc_library(
    name = "editor_lib",
    srcs = glob([
        "src/**/*.cpp",
    ]),
    hdrs = glob([
        "include/**/*.h",
    ]),
    includes = [
        "editor",
        "vshade/include",
        "include",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//vshade:vshade_lib",
        "//editor/vendors:hdf5_lib",
    ],
    # data = glob([
    #     "resources/**/*",  # Engine resources
    # ]),
    alwayslink = True,
)

# -----------------------------------------------------------
# Editor Application
# -----------------------------------------------------------
cc_binary(
    name = "editor",
    deps = [
        ":editor_lib",
        #"//vshade:vshade_lib"
    ]
)