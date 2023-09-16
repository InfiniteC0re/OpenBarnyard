IncludeDir = {}
IncludeDir.spdlog = "%{wks.location}/Toshi/Vendor/spdlog/include"
IncludeDir.fmod = "%{wks.location}/Toshi/Vendor/fmod/include"
IncludeDir.trbf = "%{wks.location}/Tools/TRBF/Include"
IncludeDir.libogg = "%{wks.location}/Toshi/Vendor/libogg/include"
IncludeDir.libvorbis = "%{wks.location}/Toshi/Vendor/libvorbis/include"
IncludeDir.libtheora = "%{wks.location}/Toshi/Vendor/libtheora/include"
IncludeDir.theoraplay = "%{wks.location}/Toshi/Vendor/theoraplay/include"
IncludeDir.stb = "%{wks.location}/Toshi/Vendor/stb"
IncludeDir.sdl2 = "%{wks.location}/Toshi/Vendor/sdl2/include"
IncludeDir.glm = "%{wks.location}/Toshi/Vendor/glm/include"
IncludeDir.glew = "%{wks.location}/Toshi/Vendor/glew/include"

LibDir = {}
LibDir.fmod = "%{wks.location}/Toshi/Vendor/fmod/lib"
LibDir.sdl2 = "%{wks.location}/Toshi/Vendor/sdl2/lib"
LibDir.glew = "%{wks.location}/Toshi/Vendor/glew/lib"

-- content of these folders should be copied to any client application
ClientContentCommon = "%{wks.location}Content/Common/"
ClientContentArch   = "%{wks.location}Content"

-- modify paths based on architecture
LibDir.fmod = LibDir.fmod .. "/" .. _OPTIONS["arch"] .. "/"
LibDir.sdl2 = LibDir.sdl2 .. "/" .. _OPTIONS["arch"] .. "/"
LibDir.glew = LibDir.glew .. "/" .. _OPTIONS["arch"] .. "/"
ClientContentArch = ClientContentArch .. "/" .. _OPTIONS["arch"] .. "/"