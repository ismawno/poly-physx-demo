project "poly-physx-demo"
   kind "ConsoleApp"
   staticruntime "off"
   defines {'ROOT_PATH="' .. rootpath .. '"'}

   language "C++"
   cppdialect "C++17"
   filter "system:macosx"
      buildoptions {"-Wall", "-Wextra", "-Wpedantic", "-Wconversion", "-Wno-unused-parameter"}
   filter{}

   targetdir("bin/" .. outputdir)
   objdir("build/" .. outputdir)

   files {"src/**.cpp", "include/**.hpp"}
   includedirs {"../**/include", "../vendor/glm"}

   links {"profile-tools", "ini-parser", "rk-integrator", "shapes-2D", "poly-physx", "imgui", "imgui-SFML", "implot", "poly-physx-app", "sfml-primitives"}
   libdirs "../vendor/SFML/build-sfml/lib"
   
   filter "system:macosx"
      links {"sfml-graphics", "sfml-window", "sfml-system", "OpenGL.framework"}
   filter "system:windows"
      
      links {"opengl32", "freetype", "winmm", "gdi32"}
      filter {"system:windows", "configurations:debug*"}
         libdirs "../vendor/SFML/build-sfml/lib/Debug"
         links {"sfml-graphics-s-d", "sfml-window-s-d", "sfml-system-s-d"}
         
      filter {"system:windows", "configurations:release*"}
         libdirs "../vendor/SFML/build-sfml/lib/Release"
         links {"sfml-graphics-s", "sfml-window-s", "sfml-system-s"}

      filter {"system:windows", "platforms:x86_64", "action:vs*"}
         libdirs "../vendor/SFML/extlibs/libs-msvc/x64"
         
      filter {"system:windows", "platforms:x86", "action:vs*"}
         libdirs "../vendor/SFML/extlibs/libs-msvc/x86"

      filter {"system:windows", "platforms:x86_64", "action:gmake*"}
         libdirs "../vendor/SFML/extlibs/libs-mingw/x64"
         
      filter {"system:windows", "platforms:x86", "action:gmake*"}
         libdirs "../vendor/SFML/extlibs/libs-mingw/x86"
