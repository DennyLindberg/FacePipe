function findLatestPython(path, desired_version)
    path = path:gsub("\\","/") -- \ to /

    python_dir = ""
    python_version = ""

    command = "dir \"" .. path .. "\" /b /ad"
    for dir in io.popen(command):lines() do 
        -- folders are iterated in name order (keep picking the latest to get the greatest version of python)
        if string.find(dir, "python") or string.find(dir, "Python") then
            python_dir = path .. dir
            python_version =  string.lower(dir)

            if python_version == desired_version then
                return python_dir, python_version
            end
        end
    end

    return python_dir, python_version
end

function getPythonPathAndVersion(desired_version)
    python_dir = ""
    python_version = ""
    
    localappdata = os.getenv('LOCALAPPDATA') .. "\\Programs\\Python\\" -- C:\Users\name\AppData\Local\Programs\
    python_dir, python_version = findLatestPython(localappdata, desired_version)
    
    if python_version ~= desired_version then
        program_files = 'C:/Program Files/'
        python_dir, python_version = findLatestPython(program_files, desired_version)
    end

    return python_dir, python_version
end

function setupPython(expected_version)
    python_path, version = getPythonPathAndVersion(expected_version)

    if version ~= expected_version then
        error("Failed to find correct python! Expected " .. expected_version)
    end

    python_includes_folder  = python_path .. "/include/"
    python_libs_folder      = python_path .. "/libs/"
    python_lib              = version .. ".lib"
    print("Python includes: " .. python_includes_folder)
    print("Python libs: " .. python_libs_folder)
    print("lib: " .. python_lib)

    includedirs { python_includes_folder }
    libdirs { python_libs_folder }
    links   { python_lib }
end

---
-- Solution
---
python_enable = "0"
python_multithreaded = "0"
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
        "TINYOBJLOADER_IMPLEMENTATION",
        "PYTHON_ENABLED="..python_enable,
        "PYTHON_MULTITHREADED="..python_multithreaded
    }        

    filter { "platforms:*64"} architecture "x64"

    symbolspath '$(TargetName).pdb'
    staticruntime "on"

    debugdir(binaries_folder)
    includedirs { includes_folder, source_folder, source_thirdparty_folder }
    libdirs     { libs_folder }
    links       { "opengl32", "SDL2", "ws2_32.lib" }
    flags       { "MultiProcessorCompile" }

    if python_enable == "1" then
        setupPython("python311")
    end

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