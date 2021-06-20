workspace "NetworkDemo"
   architecture "x86_64"
   startproject "NetServer"
configurations { "Debug", "Release" }



 project "NetCommon"
   kind "StaticLib"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"
   flags { "NoPCH" }
   files { "./NetCommon/**.h", "./NetCommon/**.cpp" }
   includedirs {"./vendor/asio/include"}
   filter "configurations:Debug"
      defines { "DEBUG", "ASIO_STANDALONE" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG", "ASIO_STANDALONE" }
      optimize "On"


 project "NetServer"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"
   flags { "NoPCH" }
   files { 
         "./NetServer/**.h", 
         "./NetServer/**.cpp" 
         }
   includedirs {
      "./vendor/asio/include",
      "./NetCommon/",
      "./NetServer/"
      }
   filter "configurations:Debug"
      defines { "DEBUG",  "ASIO_STANDALONE" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG",  "ASIO_STANDALONE" }
      optimize "On"

project "NetClient"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"
   flags { "NoPCH" }
   files { "./NetClient/**.h", "./NetClient/**.cpp"  }
   includedirs {
      "./vendor/asio/include", 
      "./NetCommon/",
      "./NetClient"
   }
   filter "configurations:Debug"
      defines { "DEBUG",  "ASIO_STANDALONE" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG",  "ASIO_STANDALONE" }
      optimize "On"



 project "GetWebPage"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"
   flags { "NoPCH" }
   files { "./ConsoleApp/**.h", "./ConsoleApp/**.cpp" }
   includedirs {"./vendor/asio/include"}
   filter "configurations:Debug"
      defines { "DEBUG", "ASIO_STANDALONE" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG", "ASIO_STANDALONE" }
      optimize "On"

      