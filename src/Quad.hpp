#include <GL/glew.h>

class Quad {
public:
	Quad();
	Quad(const Quad&) = delete;
	Quad& operator=(const Quad&) = delete;

	void Begin();
	void Draw();
	void End();

private:
	GLuint index_buffer;
	GLuint vertex_buffer;
	GLuint vertex_array;
};