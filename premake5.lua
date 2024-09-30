workspace "FlappyBird"
    configurations { 'Debug', 'Release' }
    platforms { 'Win64' }
    location 'build'

project 'flappy-bird'
    kind 'WindowedApp'
    language 'C++'
    cppdialect 'C++20'
    files { 'src/main.cpp' }

	-- Copy assets to output directory
	postbuildcommands {
		'{MKDIR} %[%{!cfg.buildtarget.directory}/assets]',
		'{COPYDIR} %[./assets] %[%{!cfg.buildtarget.directory}/assets]'
	}

    filter 'configurations:Release'
        optimize 'On'
        defines { 'NDEBUG' }

    filter 'configurations:Debug'
        symbols 'On'

    filter 'platforms:Win64'
        system 'Windows'
        architecture 'x86_64'

	filter 'system:Windows'
		links { 'shell32', 'opengl32' }
		defines { 'WIN32', 'UNICODE' }
		
	filter { 'system:Windows', 'configurations:Debug' }
		links { 'SDL2maind', 'SDL2d' }

	filter { 'system:Windows', 'configurations:Release' }
		links { 'SDL2', 'SDL2main' }
