#pragma once 

#include <array>

namespace Asset {
	enum class Texture_ID {
		bird,
		ground,
		pipe,
		_length
	};

	struct Texture {
		const char *location;
		int width;
		int height;
	};

	namespace _Texture_File_Locations {
		static const char *bird = "assets/bird.png";
		static const char *ground = "assets/ground.png";
		static const char *pipe = "assets/pipe.png";
	};

	inline std::array<Texture, (size_t)Texture_ID::_length> texture_data = {
		Texture { .location = _Texture_File_Locations::bird },
		Texture { .location = _Texture_File_Locations::ground },
		Texture { .location = _Texture_File_Locations::pipe }
	};

	inline Texture get_texture(Texture_ID id) {
		return texture_data[(size_t)id];
	}
};