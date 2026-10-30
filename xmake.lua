set_project("zdotool")
set_version("0.1.0")

--- DEPENDENCIES
add_repositories("mj-xrepo https://github.com/monjaris/mj-xrepo")
-- clixx is built and linked as a static library (libclixx.a).
add_requires("clixx", {configs = {shared = false}})


--- CONFIG
add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate")
set_languages("c++23")
add_cxxflags(
    "-Wall", "-Wextra",
    "-fno-exceptions", "-fno-rtti"
)
add_includedirs("include")

after_build(function (target)
    os.cp(target:targetfile(), "$(projectdir)")
end)


--- TARGETS
target("zdotool")
    set_kind("binary")
    add_files("src/*.cpp")
    add_packages("clixx")
