#pragma once

#include <stdarg.h>

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "application.hpp"
#include "array.hpp"
#include "assets.hpp"
#include "game_properties.hpp"
#include "game_state.hpp"
#include "platform.hpp"

struct Basic_Shader_Program {
	GLuint id;
	struct {
		GLint view_projection;
		GLint transform;
	} uniform_location;
};

struct Shape_Shader_Program {
	GLuint id;
	struct {
		GLint shape_type;
		GLint colour;
		GLint view_projection;
		GLint transform;
	} uniform_location;
};

struct Shader_Contents {
	const char *vertex;
	const char *fragment;
};

struct GL_Renderer {
	std::array<GLuint, static_cast<size_t>(Asset::Texture_ID::_length)> texture_indices = {};
	Basic_Shader_Program basic_shader_program;
	Shape_Shader_Program shape_shader_program;

	GL_Renderer(Platform &);

	void init(
		const Application &, 
		GLDEBUGPROC debug_message_callback,
		const Shader_Contents &basic_shader_contents, 
		const Shader_Contents &shape_shader_contents
	);
	void render(const Application &, const Game_State &) const;
	void load(unsigned char * data, Asset::Texture_ID, const Asset::Texture &);

private:
	Platform &platform;

	void create_basic_shader(const Shader_Contents &);
	void create_shape_shader(const Shader_Contents &);
	void create_vertex_data() const;
	void draw_circle(const glm::vec2 &position, const float &radius, const glm::vec4 &colour) const;
	void draw_rect(const glm::vec4 &rect, const glm::vec4 &colour) const;
	void log(const char *format, ...) const;
	void set_viewport(const Application &);
	void log_shader_compile_error(GLuint shader_id, const char *name) const;
	void log_shader_link_error(GLuint program_id, const char *name) const;
};