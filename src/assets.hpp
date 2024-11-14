#pragma once 

#include <array>

namespace Asset {
	enum class Texture_ID {
		cloud1,
		cloud2,
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
		static const char *cloud1 = "images/cloud1.png";
		static const char *cloud2 = "images/cloud2.png";
		static const char *bird = "images/bird.png";
		static const char *ground = "images/ground.png";
		static const char *pipe = "images/pipe.png";
		static const char *sky = "images/sky.png";
		static const char *hills = "images/hills.png";
	};

	inline std::array<Texture, (size_t)Texture_ID::_length> texture_data = {
		Texture { .location = _Texture_File_Locations::cloud1 },
		Texture { .location = _Texture_File_Locations::cloud2 },
		Texture { .location = _Texture_File_Locations::bird },
		Texture { .location = _Texture_File_Locations::ground },
		Texture { .location = _Texture_File_Locations::pipe },
		Texture { .location = _Texture_File_Locations::sky },
		Texture { .location = _Texture_File_Locations::hills }
	};

	inline Texture get_texture(Texture_ID id) {
		return texture_data[(size_t)id];
	}

	enum class Shader_ID {
		basic,
		shape,
		text,
		_length,
		none
	};

	namespace _Shader_File_Locations {
		static const char *basic = "shaders/basic";
		static const char *shape = "shaders/shape";
		static const char *text = "shaders/text";
	};

	inline std::array<const char *, (size_t)Shader_ID::_length> shader_data = {
		_Shader_File_Locations::basic,
		_Shader_File_Locations::shape,
		_Shader_File_Locations::text
	};

	inline const char *get_shader(Shader_ID id) {
		return shader_data[(size_t)id];
	}

	inline const char *font_location = "fonts/PressStart2P-Regular.ttf";

	enum class Audio_ID {
		flap,
		hit,
		score,
		_length,
		none
	};

	namespace _Audio_File_Locations {
		static const char *flap = "audio/flap.wav";
		static const char *hit = "audio/hit.wav";
		static const char *score = "audio/score.wav";
	};

	inline std::array<const char *, (size_t)Audio_ID::_length> audio_data = {
		_Audio_File_Locations::flap,
		_Audio_File_Locations::hit,
		_Audio_File_Locations::score
	};

	inline const char *get_audio(Audio_ID id) {
		return audio_data[(size_t)id];
	}
};