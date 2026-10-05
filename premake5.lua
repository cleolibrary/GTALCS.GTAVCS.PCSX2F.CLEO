workspace "GTALCS.GTAVCS.PCSX2F.CLEO"
   configurations { "Release", "Debug" }
   platforms { "Win64" }
   architecture "x64"
   location "build"
   objdir ("build/obj")
   buildlog ("build/log/%{prj.name}.log")
   cppdialect "C++latest"
   
   kind "SharedLib"
   language "C++"
   targetdir "data/scripts"
   targetextension ".asi"
   characterset ("UNICODE")
   staticruntime "On"
   
   files { "source/*.h" }
   files { "source/*.cpp" }
   files { "Resources/*.rc" }
   includedirs { "source" }
   
   pbcommands = { 
      "setlocal EnableDelayedExpansion",
      --"set \"path=" .. (gamepath) .. "\"",
      "set file=$(TargetPath)",
      "FOR %%i IN (\"%file%\") DO (",
      "set filename=%%~ni",
      "set fileextension=%%~xi",
      "set target=!path!!filename!!fileextension!",
      "if exist \"!target!\" copy /y \"%%~fi\" \"!target!\"",
      ")" }
   
   function setbuildpaths_ps2(gamepath, exepath, scriptspath, ps2sdkpath, sourcepath, prj_name)
      local command = 'powershell -NoProfile -ExecutionPolicy Bypass -File "%{wks.location}/../external/ps2sdk/plugins/build-module.ps1" -Project "' .. sourcepath .. 'module.json"'
      local install = os.getenv("PCSX2FDir") or gamepath
      local deploy = {}
      if install and os.isdir(install) then
         local target = path.join(install, scriptspath, "cleo.elf")
         deploy = { 'if exist "' .. target .. '" copy /y "$(NMakeOutput)" "' .. target .. '"' }
      end
      buildcommands { command, 'if errorlevel 1 exit /b %errorlevel%', deploy }
      rebuildcommands { command .. ' -Clean', 'if errorlevel 1 exit /b %errorlevel%', command,
         'if errorlevel 1 exit /b %errorlevel%', deploy }
      cleancommands { command .. ' -Clean' }
      targetdir("data/" .. scriptspath)
      debugdir(gamepath)
      debugcommand(path.join(gamepath, os.isfile(path.join(gamepath, "pcsx2-qtx64.exe")) and "pcsx2-qtx64.exe" or "pcsx2-qt.exe"))
   end

   function add_ps2sdk()
      includedirs { "external/ps2sdk/ps2sdk/ee", "external/injector/include" }
      files { "external/injector/include/ps2/**.h", "external/injector/include/ps2/**.hpp" }
      files { "source/*.h", "source/*.c", "source/*.cpp", "source/makefile", "source/module.json" }
   end
      
   filter "configurations:Debug*"
      defines "DEBUG"
      symbols "On"

   filter "configurations:Release*"
      defines "NDEBUG"
      optimize "On"


project "GTALCS.GTAVCS.PCSX2F.CLEO"
   kind "Makefile"
   targetname "cleo"
   add_ps2sdk()
   targetextension ".elf"
   setbuildpaths_ps2("Z:/GitHub/PCSX2-Fork-With-Plugins/bin/", "pcsx2-qtx64-clang.exe", "PLUGINS/", "%{wks.location}/../external/ps2sdk/ee/bin/vsmake.ps1", "%{wks.location}/../source/", "cleo")
