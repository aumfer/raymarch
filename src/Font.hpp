#include <GL/glew.h>
#include <glm/glm.hpp>

enum FontStyle {
	ARIAL_16,
	CONSOLAS_16
};

struct texture_font;

class Font {
public:
	Font(FontStyle);
	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;

	void Begin();
	void Print(const char *text, const glm::vec2 &position, const glm::u8vec4 color);
	void Draw(int width, int height);
	void End();

private:
	FontStyle style;
	texture_font &data;

	GLuint texture;
	GLuint vertex_array;
	GLuint array_buffer, element_array_buffer;
	struct {
		GLint projection, font;
	} uniforms;
	struct {
		GLint color, texcoord, position;
	} attributes;
	GLuint program;
	struct {
		GLuint vertex, fragment;
	} shaders;
	GLuint glyph_count;
};