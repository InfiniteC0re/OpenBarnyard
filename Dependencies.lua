IncludeDir = {}
IncludeDir.spdlog = "%{wks.location}/Toshi/Vendor/spdlog/include"
IncludeDir.fmod = "%{wks.location}/Toshi/Vendor/fmod/include"
IncludeDir.trbf = "%{wks.location}/Tools/TRBF/Include"
IncludeDir.libogg = "%{wks.location}/Toshi/Vendor/libogg/include"
IncludeDir.libvorbis = "%{wks.location}/Toshi/Vendor/libvorbis/include"
IncludeDir.libtheora = "%{wks.location}/Toshi/Vendor/libtheora/include"
IncludeDir.theoraplay = "%{wks.location}/Toshi/Vendor/theoraplay/include"
IncludeDir.stb = "%{wks.location}/Toshi/Vendor/stb"
IncludeDir.dx8 = "%{wks.location}/Toshi/Vendor/DX81/include"

LibDir = {}
LibDir.fmod = "%{wks.location}/Toshi/Vendor/fmod/lib"
LibDir.dx8 = "%{wks.location}/Toshi/Vendor/DX81/lib"

-- content of these folders should be copied to any client application
ClientContentCommon = "%{wks.location}Content/Common/"
ClientContentArch   = "%{wks.location}Content"

-- modify paths based on architecture
LibDir.fmod = LibDir.fmod .. "/" .. _OPTIONS["arch"] .. "/"
ClientContentArch = ClientContentArch .. "/" .. _OPTIONS["arch"] .. "/"