function getPythonPathAndVersion(desired_version)
    base = 'C:/Program Files/'
    python_dir = ""
    python_version = ""
    for dir in io.popen([[dir "C:\Program Files\" /b /ad]]):lines() do 
        -- folders are iterated in name order (keep picking the latest to get the greatest version of python)
        if string.find(dir, "python") or string.find(dir, "Python") then
            python_dir = base .. dir
            python_version = "" .. dir

            if python_version == desired_version then
                break -- found the exact version we are looking for
            end
        end
    end

    return python_dir, python_version
end

python_path, version = getPythonPathAndVersion("python311")
python_includes_folder  = python_path .. "/include/"
python_libs_folder      = python_path .. "/libs/"
python_lib              = python_libs_folder .. version .. ".lib"

if python_lib == "" then
    error("Failed to find python path!")
else
    print("Python includes: " .. python_includes_folder)
    print("Python libs: " .. python_libs_folder)
    print("lib: " .. python_lib)
end

---
-- Solution
---
binaries_folder = "binaries/"
includes_folder = "include/"
source_folder   = "source/"
source_thirdparty_folder   = "source/thirdparty/"
libs_folder     = "libs/"

workspace "FacePipe"
    location("temp/") -- temporary files (sln, proj, obj, pdb, ilk, etc)
    language "C++"

    configurations { "Debug", "Release" }

    cppdialect "C++20"
    systemversion("latest")
    system      "windows"
    platforms { "win64" }
    defines   { 
        "OS_WINDOWS",
        "TINYOBJLOADER_IMPLEMENTATION"
    }        

    filter { "platforms:*64"} architecture "x64"

    symbolspath '$(TargetName).pdb'
    staticruntime "on"

    debugdir(binaries_folder)
    includedirs { includes_folder, source_folder, source_thirdparty_folder, python_includes_folder }
    libdirs     { libs_folder, python_libs_folder }
    links       { "opengl32", "SDL2", python_lib }
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
    