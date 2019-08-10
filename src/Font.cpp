#include "Font.hpp"
#include "lsgl.hpp"
#include <freetype-gl/arial-16.h>
#include <freetype-gl/consolas-16.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

typedef struct font_vertex {
	GLubvec4 color;
	GLusvec2 texcoord;
	GLvec2 position;
} font_vertex_t;
typedef GLuint font_index_t;

#define FONT_TEXTURE_UNIT 0

#define GLYPH_VERTICES 4
#define GLYPH_INDICES 6
#define MAX_GLYPHS (1024*128)

#define ARRAY_BUFFER_MAX ((MAX_GLYPHS)*(GLYPH_VERTICES))
#define ARRAY_BUFFER_SIZE (sizeof(font_vertex_t)*(ARRAY_BUFFER_MAX))

#define ELEMENT_ARRAY_BUFFER_MAX ((MAX_GLYPHS)*(GLYPH_INDICES))
#define ELEMENT_ARRAY_BUFFER_SIZE (sizeof(GLuint)*(ELEMENT_ARRAY_BUFFER_MAX))

static_assert(ARRAY_BUFFER_SIZE % 16 == 0, "array buffer size");
static_assert(ELEMENT_ARRAY_BUFFER_SIZE % 16 == 0, "element array buffer size");

static texture_font_t *font_data[] = {
	&font_arial16,
	&font_consolas16
};

static const GLchar vertex_source[] = GLSL_BEGIN GLSL(
	uniform mat4 projection;

in vec4 color;
in vec2 texcoord;
in vec2 position;

out vec4 f_color;
out vec2 f_texcoord;

void main() {
	f_color = color;
	f_texcoord = texcoord;
	gl_Position = projection * vec4(position, 0, 1);
}
);
static const GLchar fragment_source[] = GLSL_BEGIN GLSL(
	uniform sampler2D font;

in vec4 f_color;
in vec2 f_texcoord;

out vec4 p_color;

void main() {
	p_color = f_color *texture(font, f_texcoord).rrrr;
}
);

Font::Font(FontStyle _style) :
	style(_style),
	data(*font_data[_style]){

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, data.tex_width, data.tex_height);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.tex_width, data.tex_height, GL_RED, GL_UNSIGNED_BYTE, data.tex_data);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenBuffers(1, &element_array_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, ELEMENT_ARRAY_BUFFER_SIZE, NULL, GL_MAP_WRITE_BIT);

	glGenBuffers(1, &array_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
	glBufferStorage(GL_ARRAY_BUFFER, ARRAY_BUFFER_SIZE, NULL, GL_MAP_WRITE_BIT);

	shaders.vertex = glCreateShader(GL_VERTEX_SHADER);
	lsgl::compileshader(shaders.vertex, vertex_source, lsgl_countof(vertex_source));
	shaders.fragment = glCreateShader(GL_FRAGMENT_SHADER);
	lsgl::compileshader(shaders.fragment, fragment_source, lsgl_countof(fragment_source));
	program = glCreateProgram();
	glAttachShader(program, shaders.vertex);
	glAttachShader(program, shaders.fragment);
	lsgl::linkprogram(program);

	glUseProgram(program);
	uniforms.projection = glGetUniformLocation(program, "projection");
	uniforms.font = glGetUniformLocation(program, "font");
	attributes.color = glGetAttribLocation(program, "color");
	attributes.texcoord = glGetAttribLocation(program, "texcoord");
	attributes.position = glGetAttribLocation(program, "position");
	glUniform1i(uniforms.font, FONT_TEXTURE_UNIT);

	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	glVertexAttribPointer(attributes.color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(font_vertex_t), lsgl_offsetof(font_vertex_t, color));
	glEnableVertexAttribArray(attributes.color);
	glVertexAttribPointer(attributes.texcoord, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(font_vertex_t), lsgl_offsetof(font_vertex_t, texcoord));
	glEnableVertexAttribArray(attributes.texcoord);
	glVertexAttribPointer(attributes.position, 2, GL_FLOAT, GL_FALSE, sizeof(font_vertex_t), lsgl_offsetof(font_vertex_t, position));
	glEnableVertexAttribArray(attributes.position);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void Font::Begin() {
	glyph_count = 0;
}

void Font::Print(const char *text, const glm::vec2 &position, const glm::u8vec4 color) {
	size_t print_count;
	font_vertex_t *vertex;
	font_index_t *index;
	glm::vec2 pen;
	float x[2], y[2];
	size_t i;

	pen = glm::vec2(position[0], position[1]);

	print_count = strnlen_s(text, MAX_GLYPHS - glyph_count);
	assert(glyph_count + print_count < MAX_GLYPHS);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);
	index = (font_index_t *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, glyph_count*GLYPH_INDICES * sizeof(font_index_t), print_count*GLYPH_INDICES * sizeof(font_index_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	assert(index);

	glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
	vertex = (font_vertex_t *)glMapBufferRange(GL_ARRAY_BUFFER, glyph_count*GLYPH_VERTICES * sizeof(font_vertex_t), print_count*GLYPH_VERTICES * sizeof(font_vertex_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	assert(vertex);

	for (i = 0; i < print_count; ++i) {
		const texture_glyph_t *glyph = NULL;
		const texture_glyph_t *g;

		for (g = data.glyphs; g < data.glyphs + data.glyphs_count; ++g) {
			if (g->charcode == text[i]) {
				glyph = g;
				break;
			}
		}
		if (!glyph) {
			continue;
		}

		if (i > 0) {
			const kerning_t *k;
			for (k = glyph->kerning; k < glyph->kerning + glyph->kerning_count; ++k) {
				if (k->charcode == text[i - 1]) {
					pen.x += k->kerning;
				}
			}
		}

		x[0] = pen.x + glyph->offset_x;
		y[0] = pen.y + glyph->offset_y;
		x[1] = x[0] + glyph->width;
		y[1] = y[0] - glyph->height;

		vertex[0].color[0] = color.r;
		vertex[0].color[1] = color.g;
		vertex[0].color[2] = color.b;
		vertex[0].color[3] = color.a;
		vertex[0].texcoord[0] = lsgl::normalize<GLushort>(glyph->s0);
		vertex[0].texcoord[1] = lsgl::normalize<GLushort>(glyph->t0);
		vertex[0].position[0] = x[0];
		vertex[0].position[1] = y[0];

		vertex[1].color[0] = color.r;
		vertex[1].color[1] = color.g;
		vertex[1].color[2] = color.b;
		vertex[1].color[3] = color.a;
		vertex[1].texcoord[0] = lsgl::normalize<GLushort>(glyph->s0);
		vertex[1].texcoord[1] = lsgl::normalize<GLushort>(glyph->t1);
		vertex[1].position[0] = x[0];
		vertex[1].position[1] = y[1];

		vertex[2].color[0] = color.r;
		vertex[2].color[1] = color.g;
		vertex[2].color[2] = color.b;
		vertex[2].color[3] = color.a;
		vertex[2].texcoord[0] = lsgl::normalize<GLushort>(glyph->s1);
		vertex[2].texcoord[1] = lsgl::normalize<GLushort>(glyph->t1);
		vertex[2].position[0] = x[1];
		vertex[2].position[1] = y[1];

		vertex[3].color[0] = color.r;
		vertex[3].color[1] = color.g;
		vertex[3].color[2] = color.b;
		vertex[3].color[3] = color.a;
		vertex[3].texcoord[0] = lsgl::normalize<GLushort>(glyph->s1);
		vertex[3].texcoord[1] = lsgl::normalize<GLushort>(glyph->t0);
		vertex[3].position[0] = x[1];
		vertex[3].position[1] = y[0];

		index[0] = glyph_count*GLYPH_VERTICES + 0;
		index[1] = glyph_count*GLYPH_VERTICES + 1;
		index[2] = glyph_count*GLYPH_VERTICES + 2;
		index[3] = glyph_count*GLYPH_VERTICES + 0;
		index[4] = glyph_count*GLYPH_VERTICES + 2;
		index[5] = glyph_count*GLYPH_VERTICES + 3;

		pen.x += glyph->advance_x;
		pen.y += glyph->advance_y;

		vertex += GLYPH_VERTICES;
		index += GLYPH_INDICES;
		++glyph_count;
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Font::Draw(int width, int height) {
	auto projection = glm::ortho(0.0f, GLfloat(width), 0.0f, GLfloat(height));

	glViewport(0, 0, width, height);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0 + FONT_TEXTURE_UNIT);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUseProgram(program);
	glUniformMatrix4fv(uniforms.projection, 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_array_buffer);

	glDrawElements(GL_TRIANGLES, glyph_count*GLYPH_INDICES, GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);
}

void Font::End() {

}