load("@rules_cc//cc:defs.bzl", "cc_library")

config_setting(
    name = "debug_mode",
    values = {"compilation_mode": "dbg"},
)

config_setting(
    name = "release_mode",
    values = {"compilation_mode": "opt"},
)

filegroup(
    name = "engine_assets",
    srcs = glob(["resources/**"]),
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------
# Vshade Library
# -----------------------------------------------------------
cc_library(
    name = "vshade_lib",
    srcs = glob(["src/**/*.cpp"]),
    hdrs = glob(["include/**/*.h"]),
    includes = [
        "include",
        "vendors",
        "vendors/vulkan",
        "vendors/ankerl/*.h",
        "vendors/imgui",
        "vendors/spdlog/include",
    ],
    visibility = ["//visibility:public"],
    copts = select({
        "@bazel_tools//src/conditions:windows": [
            "/DGLM_ENABLE_EXPERIMENTAL",
            "/DGLM_FORCE_DEPTH_ZERO_TO_ONE"
        ],
        "@bazel_tools//src/conditions:linux_x86_64": [
            "-DGLM_ENABLE_EXPERIMENTAL",
            "-DGLM_FORCE_DEPTH_ZERO_TO_ONE",
            "-Wall",
            "-Werror=unused-variable",
            "-Wextra",
        ],
        "//conditions:default": [],
    }) + select({
        ":debug_mode": [
            "-D_VSHADE_DEBUG_",
        ],
        "//conditions:default": [],
    }),
    linkopts = select({
        ":debug_mode": [
            "-Wl,-t",
        ],
        ":release_mode": [
            "-Wl,-t",
        ],
        "//conditions:default": [],
    }),
    deps = [
        "//vshade/vendors:vulkan_lib",
        "//vshade/vendors:glfw",
        "//vshade/vendors:spdlog",
        "//vshade/vendors:ankerl",
        "//vshade/vendors:glm",
        "//vshade/vendors:imgui",
        "//vshade/vendors:entt",
    ],
    data = [
        ":engine_assets",
    ],
)