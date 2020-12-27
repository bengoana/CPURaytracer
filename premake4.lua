
solution ("RayTracer" .. _ACTION)
	configurations { "Debug", "Release" }
	platforms { "x32", "x64" }
	language "C++"
	location ("build")	

  includedirs {
    "./include",
    "./deps/include",
    "deps",
  }  

  project "RecursiveTracer"
  kind "ConsoleApp"
	targetname ("RayTracer")
	language "C++"
  prj_path = "./build/" .. "RayTracer"

  location (prj_path .. "/" .. _ACTION)

  defines { "_CRT_SECURE_NO_WARNINGS", "_GLFW_WIN32", "WITH_MINIAUDIO", "MTR_ENABLED" }
  flags { "ExtraWarnings" }

  configuration "vs2019"
    windowstargetplatformversion "10.0.18362.0"
    --windowstargetplatformversion "10.0.17763.0"
  
  configuration "Debug"
    defines { "DEBUG" }
    targetdir ("bin/Debug")
    targetsuffix "_d"
    objdir ("build/" .. "Debug")
    flags { "Symbols", "NoPCH" }
    links{
      "./deps/lib/x86/*"
    }

  configuration "Release"
    targetdir ("bin/Release")
    objdir ("build/" .. "Release")
    flags { "Optimize", "NoPCH" }
    links{
      "./deps/lib/x86/*"
    }
  

  project "RecursiveTracer"
  files {
    --GLM
    "./deps/glm/**.h",
    "./deps/glm/**.inl",
    "./deps/px_sched.h",
    "./deps/minitrace.h",
    "./deps/minitrace.c",
    "./deps/tiny_obj_loader.h",
    "./src/*.cc",
    "./include/*.h"


  }


