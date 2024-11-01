function assert(condition, message)
	if not condition then
		error(message or 'Assertion failed')
	end
end

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

	local include_dir = os.getenv('INCLUDE_DIR')
	assert(include_dir ~= nil, 'INCLUDE_DIR environment variable has not been defined.')
	includedirs { include_dir }

    filter 'configurations:Release'
		local lib_dir = os.getenv('LIB_DIR')
		assert(lib_dir ~= nil, 'LIB_DIR environment variable has not been defined.')
		libdirs { lib_dir } 

        optimize 'On'
        defines { 'NDEBUG' }

    filter 'configurations:Debug'
		local debug_lib_dir = os.getenv('DEBUG_LIB_DIR')
		libdirs { debug_lib_dir or lib_dir }

        symbols 'On'

    filter 'platforms:Win64'
        system 'Windows'
        architecture 'x86_64'

	filter 'system:Windows'
		links { 'shell32', 'opengl32' }
		defines { 'WIN32', 'UNICODE' }
		
	filter { 'system:Windows', 'configurations:Debug' }
		links { 'glew32d', 'SDL2maind', 'SDL2d', 'zlibd', 'bz2d', 'brotlicommon', 'brotlidec', 'libpng16d', 'freetyped' }

	filter { 'system:Windows', 'configurations:Release' }
		links { 'glew32', 'SDL2main', 'SDL2', 'zlib', 'bz2', 'brotlicommon', 'brotlidec', 'libpng16', 'freetype' }
