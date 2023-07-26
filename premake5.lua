---
-- Python path and libs
---
function queryTerminal(command)
    local success, handle = pcall(io.popen, command)
    if not success then 
        return ""
    end

    result = handle:read("*a")
    handle:close()
    result = string.gsub(result, "\n$", "") -- remove trailing whitespace
    return result
end

function getPythonPath()
    local p = queryTerminal('python -c "import sys; import os; print(os.path.dirname(sys.executable))"')
    
    -- sanitize path before returning it
    p = string.gsub(p, "\\\\", "\\") -- replace double backslash
    p = string.gsub(p, "\\", "/") -- flip slashes
    return p
end

function getPythonLib()
    return queryTerminal("python -c \"import sys; import os; import glob; path = os.path.dirname(sys.executable); libs = glob.glob(path + '/libs/python*'); print(os.path.splitext(os.path.basename(libs[-1]))[0]);\"")
end

python_includes_folder  = getPythonPath() .. "/include/"
python_libs_folder      = getPythonPath() .. "/libs/"
python_lib              = getPythonLib()

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
    