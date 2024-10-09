workspace "GTALCS.GTAVCS.PCSX2F.CLEO"
   configurations { "Release", "Debug" }
   platforms { "Win64" }
   architecture "x64"
   location "build"
   objdir ("build/obj")
   buildlog ("build/log/%{prj.name}.log")
   cppdialect "C++latest"
   include "makefile.lua"
   
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
      local pbcmd = {}
      for k,v in pairs(pbcommands) do
        pbcmd[k] = v
      end
      if (gamepath) then
         cmdcopy = { "set \"path=" .. gamepath .. scriptspath .. "\"" }
         pbcmd[2] = "set \"file=../data/" .. scriptspath .. prj_name ..".elf\""
         table.insert(cmdcopy, pbcmd)
         buildcommands   { "call " .. ps2sdkpath .. " -C " .. sourcepath, cmdcopy }
         rebuildcommands { "call " .. ps2sdkpath .. " -C " .. sourcepath .. " clean && " .. ps2sdkpath .. " -C " .. sourcepath, cmdcopy }
         cleancommands   { "call " .. ps2sdkpath .. " -C " .. sourcepath .. " clean" }
         debugdir (gamepath)
         if (exepath) then
            debugcommand (gamepath .. exepath)
            dir, file = exepath:match'(.*/)(.*)'
            debugdir (gamepath .. (dir or ""))
         end
      end
      targetdir ("data/" .. scriptspath)
   end

   function add_ps2sdk()
      includedirs { "external/ps2sdk/ps2sdk/ee" }
      files { "source/*.h", "source/*.c", "source/*.cpp", "source/makefile" }
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
   setbuildpaths_ps2("Z:/GitHub/PCSX2-Fork-With-Plugins/bin/", "pcsx2-qtx64-clang.exe", "PLUGINS/", "%{wks.location}/../external/ps2sdk/ee/bin/vsmake", "%{wks.location}/../source/", "cleo")
   writemakefile_ps2("cleo", "PLUGINS/", "0x05000000", "-l:libstdc++.a -l:libm.a",
   "../includes/pcsx2/memalloc.o", "../includes/pcsx2/log.o", "utils.o", "text.o", "libres.o", "armhook.o", "touch.o", "ui.o", "pattern.o", "core.o", "mutex.o", "strutils.o", "memutils.o", "psplang.o")
   writelinkfile_ps2()
