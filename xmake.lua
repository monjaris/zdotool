set_project("zdotool")

--- DEPENDENCIES
add_repositories("mj-xrepo https://github.com/monjaris/mj-xrepo")
add_requires("clixx")  -- for parsing command line


--- CONFIG
add_cxxflags(
    "-std=c++23",
    "-fno-exceptions", "-fno-rtti"
)

after_build(function(package)
    os.cp(package:targetfile(), "$(projectdir)")
end)


--- TARGETS
target("zdotool")
    add_includedirs("include/")
    add_files("src/*.cpp")
    add_packages("clixx")
