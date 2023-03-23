binaries_folder = "binaries/"
includes_folder = "include/"
source_folder   = "source/"
source_thirdparty_folder   = "source/thirdparty/"
libs_folder     = "libs/"

workspace "FacePipe"
    location("temp/") -- temporary files (sln, proj, obj, pdb, ilk, etc)
    language "C++"

    configurations { "Debug", "Release" }

    cppdialect "C++17"
    systemversion("latest")
    system      "windows"
    platforms { "win64" }
    defines   { 
        "OS_WINDOWS",
        "TINYOBJLOADER_IMPLEMENTATION"
    }        

    filter { "platforms:*64"} architecture "x64"

    entrypoint "mainCRTStartup"     -- force Windows-executables to use main instead of WinMain as entry point   
    symbolspath '$(TargetName).pdb'
    staticruntime "on"

    debugdir(binaries_folder)
    includedirs { includes_folder, source_folder, source_thirdparty_folder }
    libdirs     { libs_folder }
    links       { "opengl32", "SDL2" }
    flags       { "MultiProcessorCompile" }

    filter { "configurations:Debug" }
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
        
    filter { "configurations:Release" }
        defines { "NDEBUG" }
        symbols "Off"        
        optimize "On"
        
    filter{}


project "FacePipeApp"
    kind "ConsoleApp"
    targetdir(binaries_folder)
    targetname("main")
    files ({source_folder .. "**.h", source_folder .. "**.c", source_folder .. "**.cpp"})
    removefiles{ source_folder .. "main*.cpp"}
    files ({source_folder .. "main.cpp"})
