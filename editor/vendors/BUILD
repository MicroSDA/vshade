
config_setting(
    name = "windows",
    constraint_values = ["@platforms//os:windows"],
)

config_setting(
    name = "linux",
    constraint_values = ["@platforms//os:linux"],
)

#-----------------------------------------------------------
# HDF5 
#-----------------------------------------------------------
#-----------------------------------------------------------
# Linux dynamic libraries
#-----------------------------------------------------------
filegroup(
    name = "hdf5_linux_files",
    srcs = glob(["HDF5/lib/*.so*"], allow_empty = True),
    visibility = ["//visibility:public"],
)

#-----------------------------------------------------------
# Windows dynamic libraries
#-----------------------------------------------------------
filegroup(
    name = "hdf5_windows_files",
    srcs = glob(["HDF5/lib/plugin/*.dll*"], allow_empty = True),
    visibility = ["//visibility:public"],
)

alias(
    name = "hdf5-file-group",
    actual = select({
        ":linux": ":hdf5_linux_files",
        ":windows": ":hdf5_windows_files",
        "//conditions:default": ":hdf5_linux_files",
    }),
    visibility = ["//visibility:public"],
)

#-----------------------------------------------------------
# HDF5 Library
#-----------------------------------------------------------
cc_library(
    name = "hdf5_lib",
    hdrs = glob(["HDF5/include/**/*.h"]),
    includes = ["HDF5/include"],
    strip_include_prefix = "HDF5/include",
    linkstatic = True,
    alwayslink = True,
    visibility = ["//visibility:public"],

    srcs = select({
        ":linux": [
            "HDF5/lib/libhdf5.so",
            "HDF5/lib/libhdf5_hl.so",
            "HDF5/lib/libhdf5_cpp.so",
        ],
        ":windows": [
            "HDF5/lib/libhdf5.lib",
            "HDF5/lib/libhdf5_cpp.lib",
            "HDF5/lib/libhdf5_hl.lib",
            "HDF5/lib/libhdf5_hl_cpp.lib",
        ],
        "//conditions:default": [],
    }),

    linkopts = select({
        ":linux": [
            "-Wl,-rpath,$$ORIGIN",
            "-lz",
            "-ldl",
        ],
        ":windows": [
            "Shlwapi.lib",
        ],
        "//conditions:default": [],
    }),
)
