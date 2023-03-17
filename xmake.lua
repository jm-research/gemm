set_project("gemm")
set_languages("c++17")
set_warnings("allextra")

set_config("cc", "clang")
set_config("cxx", "clang++")
set_config("ld", "clang++")

add_rules("mode.debug", "mode.release")

add_includedirs("./x86")

target("gemm")
    set_kind("binary")
    add_files("x86/*.cpp")