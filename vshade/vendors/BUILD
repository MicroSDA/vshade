load("@rules_cc//cc:defs.bzl", "cc_library")

config_setting(
    name = "linux_debug",
    values = {"compilation_mode": "dbg"},
    constraint_values = ["@platforms//os:linux"],
)

config_setting(
    name = "linux_release",
    values = {"compilation_mode": "opt"},
    constraint_values = ["@platforms//os:linux"],
)

config_setting(
    name = "windows_debug",
    values = {"compilation_mode": "dbg"},
    constraint_values = ["@platforms//os:windows"],
)

config_setting(
    name = "windows_release",
    values = {"compilation_mode": "opt"},
    constraint_values = ["@platforms//os:windows"],
)

# -----------------------------------------------------------
# Vulkan Library
# -----------------------------------------------------------
cc_library(
    name = "vulkan_lib",
    hdrs = glob([
        "vulkan/include/vulkan/**/*.h",
        "vulkan/include/vulkan/**/*.hpp",
    ]),
    includes = ["vulkan/include"],
    linkstatic = True,
    visibility = ["//visibility:public"],
    srcs = select({
        ":linux_debug": [
            "vulkan/lib/libvulkan.so",
            "vulkan/lib/libshaderc_shared.so",
            "vulkan/lib/libspirv-cross-glsl.a",
            "vulkan/lib/libspirv-cross-core.a",
        ],
        ":linux_release": [
            "vulkan/lib/libvulkan.so",
            "vulkan/lib/libshaderc_shared.so",
            "vulkan/lib/libspirv-cross-glsl.a",
            "vulkan/lib/libspirv-cross-core.a",
        ],
        ":windows_debug": [
            "vulkan/lib/vulkan-1.lib",
            "vulkan/lib/shaderc_sharedd.lib",
            "vulkan/lib/spirv-cross-glsld.lib",
            "vulkan/lib/spirv-cross-cored.lib",
        ],
        ":windows_release": [
            "vulkan/lib/vulkan-1.lib",
            "vulkan/lib/shaderc_shared.lib",
            "vulkan/lib/spirv-cross-glsl.lib",
            "vulkan/lib/spirv-cross-core.lib",
        ],
        "//conditions:default": [],
    }),

    linkopts = select({
        ":linux_debug": [
            "-Wl,-rpath,$$ORIGIN",
            "-lz",
            "-ldl",
        ],
        ":linux_release": [
            "-Wl,-rpath,$$ORIGIN",
            "-lz",
            "-ldl",
        ],
        ":windows_debug": [],
        ":windows_release": [],
        "//conditions:default": [],
    }),
)

# -----------------------------------------------------------
# GLFW Library
# -----------------------------------------------------------
cc_library(
    name = "glfw",
    srcs = select({
        "@platforms//os:linux": [
            "glfw/src/context.c",      "glfw/src/init.c",          "glfw/src/input.c",         "glfw/src/monitor.c",
            "glfw/src/vulkan.c",       "glfw/src/window.c",        "glfw/src/platform.c",      "glfw/src/null_init.c",
            "glfw/src/null_monitor.c", "glfw/src/null_window.c",   "glfw/src/null_joystick.c", "glfw/src/x11_init.c",
            "glfw/src/x11_monitor.c",  "glfw/src/x11_window.c",    "glfw/src/xkb_unicode.c",   "glfw/src/posix_time.c",
            "glfw/src/posix_thread.c", "glfw/src/glx_context.c",   "glfw/src/egl_context.c",   "glfw/src/osmesa_context.c",
            "glfw/src/linux_joystick.c","glfw/src/posix_module.c", "glfw/src/posix_poll.c",
        ],
        "@platforms//os:windows": [
            "glfw/src/context.c",      "glfw/src/init.c",          "glfw/src/input.c",         "glfw/src/monitor.c",
            "glfw/src/vulkan.c",       "glfw/src/window.c",        "glfw/src/platform.c",      "glfw/src/null_init.c",
            "glfw/src/null_monitor.c", "glfw/src/null_window.c",   "glfw/src/null_joystick.c", "glfw/src/win32_init.c",
            "glfw/src/win32_joystick.c","glfw/src/win32_monitor.c", "glfw/src/win32_time.c",    "glfw/src/win32_thread.c",
            "glfw/src/win32_window.c", "glfw/src/wgl_context.c",   "glfw/src/egl_context.c",   "glfw/src/osmesa_context.c",
            "glfw/src/win32_module.c",
        ],
        "//conditions:default": [],
    }),
    hdrs = glob([
        "glfw/include/**/*.h",
        "glfw/src/**/*.h",
    ]),
    includes = [
        "glfw/include",
        "glfw/src",
    ],
    linkopts = select({
        "@platforms//os:windows" : [
            "gdi32.lib",
            "user32.lib",
            "shell32.lib",
            "kernel32.lib",
            "ole32.lib",
        ],
        "@platforms//os:linux" : []}),
    defines = select({
        "@platforms//os:linux": [
            "_GLFW_X11",
        ],
        "@platforms//os:windows": [
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS",
        ],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------
# SPDLOG Library
# -----------------------------------------------------------
cc_library(
    name = "spdlog",
    srcs = glob(["spdlog/src/**/*.cpp"]),
    hdrs = glob(["spdlog/include/spdlog/**/*.h"]),
    includes = ["spdlog/include"],
    copts = [
        "-DSPDLOG_COMPILED_LIB",
        "-utf-8",
    ],
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------
# GLM Library
# -----------------------------------------------------------
cc_library(
    name = "glm",
    hdrs = glob([
        "glm/**/*.hpp",
        "glm/**/*.h",
        "glm/**/*.inl",
    ]),
    includes = ["glm"],
    copts = ["-DGLM_ENABLE_EXPERIMENTAL"],
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------
# ANKERL Library
# -----------------------------------------------------------
cc_library(
    name = "ankerl",
    hdrs = glob(["ankerl/**/*.h"]),
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------
# IMGUI Library
# -----------------------------------------------------------
cc_library(
    name = "imgui",
    includes = ["imgui"],
    include_prefix = "imgui",
    srcs = [
        "imgui/imgui.cpp",
        "imgui/imgui_draw.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/imgui_demo.cpp",
        "imgui/imgui_tables.cpp",
        "imgui/backends/imgui_impl_glfw.cpp",
        "imgui/backends/imgui_impl_vulkan.cpp",
    ],
    hdrs = [
        "imgui/imconfig.h",
        "imgui/imgui.h",
        "imgui/imgui_internal.h",
        "imgui/imstb_rectpack.h",
        "imgui/imstb_textedit.h",
        "imgui/imstb_truetype.h",
        "imgui/backends/imgui_impl_glfw.h",
        "imgui/backends/imgui_impl_vulkan.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":glfw",
        ":vulkan_lib",
    ],
)
# -----------------------------------------------------------
# ENTT Library
# -----------------------------------------------------------
cc_library(
    name = "entt",
    hdrs = ["entt/entt.hpp"],
    visibility = ["//visibility:public"],
)

# -----------------------------------------------------------
# Assimp Library
# -----------------------------------------------------------

# cc_library(
#     name = "assimp_code_importer_lib",
#     strip_include_prefix = "assimp/include",
#     hdrs = [
#         "assimp/include/assimp/BaseImporter.h"],
#     # srcs = [
#     #     "assimp/code/**/*.cpp"
#     # ],
#     visibility = ["//visibility:public"],
# )
# cc_library(
#     name = "assimp_code_lib",
#     strip_include_prefix = "assimp/code",
#     hdrs = glob([
#         "assimp/code/**/*.h"
#     ]),
#     srcs = glob([
#         "assimp/code/**/*.cpp"
#     ]),
#     deps =[":assimp_code_importer_lib"],
#     visibility = ["//visibility:public"],
# )

# cc_library(
#     name = "assmip",
#     #include_prefix = "assimp/include",
#     strip_include_prefix = ["assimp/include", "assimp/code"],
#     hdrs = glob([
#         "assimp/include/assimp/**/*.hpp",
#         "assimp/include/assimp/**/*.h",
#         "assimp/include/assimp/**/*.inl",
#         "assimp/code/**/*.h",
#     ],
#     exclude = [
#         "**/*.md",
#         "**/CMakeLists.txt",
#         "**/*.txt",
#         "**/LICENSE",
#         "**/*.in",
#         "**/*.cmakein",
#     ]),
#     srcs = [
#             #"assimp/code/res/assimp.rc",
#             "assimp/contrib/clipper/clipper.hpp",
#             "assimp/contrib/clipper/clipper.cpp",
#         ] + glob([
#             "assimp/code/AssetLib/**",
#             "assimp/code/Common/**",
#             "assimp/code/CApi/**",
#             "assimp/code/Geometry/**",
#             "assimp/code/Material/**",
#             "assimp/code/PostProcessing/**",
#             "assimp/contrib/Open3DGC/**",
#             "assimp/contrib/openddlparser/include/openddlparser/**",
#             "assimp/contrib/openddlparser/code/**",
#             "assimp/contrib/poly2tri/poly2tri/**",
#             "assimp/contrib/pugixml/src/**/*.hpp",
#             "assimp/contrib/stb/**",
#             "assimp/contrib/unzip/**/*.h",
#             "assimp/contrib/unzip/**/*.c",
#             "assimp/contrib/zip/src/**",
#             "assimp/contrib/zlib/*",
#             "assimp/include/assimp/*.h",
#             "assimp/include/assimp/*.hpp",
#             "assimp/include/assimp/*.inl",
#         ], exclude = [
#             "**/*.md",
#             "**/CMakeLists.txt",
#             "**/*.txt",
#             "**/LICENSE",
#             "**/*.in",
#             "**/*.cmakein",
#             "**/*ChangeLog",
#             "**/*README",
#             "**/*zconf.h.included"
#     ],),
#     visibility = ["//visibility:public"],
# )