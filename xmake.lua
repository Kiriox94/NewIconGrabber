add_repositories("xmake-switch-repo https://github.com/Kiriox94/xmake-switch-repo.git")
add_repositories("zeromake-repo https://github.com/zeromake/xrepo.git")

includes("toolchain/*.lua")

add_defines(
    'BRLS_RESOURCES="romfs:/"',
    "YG_ENABLE_EVENTS",
    "STBI_NO_THREAD_LOCALS"
)

add_rules("mode.debug", "mode.release")

add_requires("borealis", "libnx", "libcurl", "lunasvg", "nxtc")

target("NewIconGrabber")
    set_kind("binary")
    if not is_plat("cross") then 
        return
    end

    add_cxxflags("-fpermissive") 
    set_arch("aarch64")
    add_rules("switch")
    set_toolchains("devkita64")
    set_languages("c++20")
    
    set_values("switch.name", "NewIconGrabber")
    set_values("switch.author", "Kiriox")
    set_values("switch.version", "1.0.0")
    set_values("switch.romfs", "resources")
    set_values("switch.icon", "resources/img/icon.jpg")

    add_files("source/**.cpp")
    add_includedirs("include")
    add_packages("borealis", "libnx", "libcurl", "lunasvg", "nxtc")