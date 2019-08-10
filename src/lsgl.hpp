#ifndef LS_LSGL_H
#define LS_LSGL_H

#include <GL/glew.h>
#include <stdlib.h>

#define GLSL_DEFINE(x) "\n"x"\n"
#define GLSL_BEGIN GLSL_DEFINE("#version 430") GLSL_DEFINE("#define M_PI 3.14159265358979323846264338327950288") GLSL_DEFINE("#define FLT_EPSILON 9.99999974737875E-06")
#define GLSL(...) #__VA_ARGS__
#define lsgl_offsetof(s,m) ((const GLvoid *)offsetof(s,m))
	//#define lsgl_countof(x) _countof(x)
#define lsgl_countof(x) (sizeof(x) / sizeof(x[0]))

#ifdef _WIN32
#  define lsgl_align(...) __declspec(align(__VA_ARGS__))
#else
#  define lsgl_align(...) __attribute__((aligned(__VA_ARGS__)))
#endif

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            lsgl::checkerror(); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

typedef GLfloat GLvec2[2];
typedef GLfloat GLvec3[3];
typedef GLfloat GLvec4[4];

typedef GLfloat GLmat4x4[4 * 4];
typedef GLmat4x4 GLmat4;

typedef GLint GLivec2[2];
typedef GLint GLivec3[3];
typedef GLint GLivec4[4];

typedef GLuint GLuvec2[2];
typedef GLuint GLuvec3[3];
typedef GLuint GLuvec4[4];

typedef GLubyte GLubvec4[4];

typedef GLushort GLusvec2[2];

namespace lsgl {
	void compileshader(GLuint shader, const char *src, int length);
	void linkprogram(GLuint program);
	void checkerror(void);
	GLint checkprogram(GLuint program);

	template<typename T>
	T normalize(GLfloat v) {
		return (T)(v * std::numeric_limits<T>().max());
	}
}
#endif