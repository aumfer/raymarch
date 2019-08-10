#include "Quad.hpp"
#include "lsgl.hpp"
#include <climits>

static GLbyte vertices[] = {
	SCHAR_MIN, SCHAR_MIN,
	SCHAR_MIN, SCHAR_MAX,
	SCHAR_MAX, SCHAR_MIN,
	SCHAR_MAX, SCHAR_MAX
};
static GLubyte indices[] = {
	0, 1, 3,
	0, 3, 2
};

Quad::Quad() {
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, 0);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	glVertexAttribPointer(0, 2, GL_BYTE, GL_TRUE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Quad::Begin() {
	glBindVertexArray(vertex_array);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
}

void Quad::Draw() {
	glDrawElements(GL_TRIANGLES, lsgl_countof(indices), GL_UNSIGNED_BYTE, NULL);
}

void Quad::End() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}