#pragma once 

#include <array>

namespace Asset {
	enum class Texture_ID {
		bird,
		ground,
		pipe,
		sky,
		hills,
		_length,
		none
	};

	struct Texture {
		const char *location;
		int width;
		int height;
	};

	namespace _Texture_File_Locations {
		static const char *bird = "assets/images/bird.png";
		static const char *ground = "assets/images/ground.png";
		static const char *pipe = "assets/images/pipe.png";
		static const char *sky = "assets/images/sky.png";
		static const char *hills = "assets/images/hills.png";
	};

	inline std::array<Texture, (size_t)Texture_ID::_length> texture_data = {
		Texture { .location = _Texture_File_Locations::bird },
		Texture { .location = _Texture_File_Locations::ground },
		Texture { .location = _Texture_File_Locations::pipe },
		Texture { .location = _Texture_File_Locations::sky },
		Texture { .location = _Texture_File_Locations::hills }
	};

	inline Texture get_texture(Texture_ID id) {
		return texture_data[(size_t)id];
	}
};