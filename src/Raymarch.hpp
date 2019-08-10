#include <GL/glew.h>

class Raymarch {
public:
	Raymarch();
	Raymarch(const Raymarch&) = delete;
	Raymarch& operator=(const Raymarch&) = delete;

	void Begin();
	void Draw(int width, int height);
	void End();

private:
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;

	struct {
		GLint random_seed;
		GLint resolution;
	} uniforms;
};