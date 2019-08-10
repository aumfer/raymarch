#include "Raymarch.hpp"
#include "lsgl.hpp"

static GLchar vertex_src[] = GLSL_BEGIN GLSL(
in vec2 position;
out vec2 pixel;
void main() {
	pixel = position;
	gl_Position = vec4(position, 0, 1);
}
);

static GLchar fragment_src[] = GLSL_BEGIN
#include "glsl/random.inl"
#include "glsl/math.inl"
GLSL(
in vec2 pixel;
out vec4 outColor;

uniform ivec2 resolution;
uniform float shutterAperture = 0.6;
uniform float fov = 2.5;
uniform float focusDistance = 1.3;
uniform float blurAmount = 0.0015;

struct Sphere {
	vec3 p;
	float r;
	vec3 c;
	vec3 e;
};
Sphere spheres[] = Sphere[](
	Sphere(vec3(0, 0, 0), 1, vec3(0), vec3(1)),
	Sphere(vec3(+2, 0, 0), 0.5, vec3(0.02, 0.2, 0.02), vec3(0)),
	Sphere(vec3(-2, 0, 0), 0.5, vec3(0.02, 0.02, 0.2), vec3(0))
);

const int num_rays = 16;
const int num_bounces = 2;

float isect_sphere(vec3 ro, vec3 rd, vec3 sp, float sr) {
	vec3 oc = ro - sp;
	float b = dot(oc, rd);
	float c = dot(oc, oc) - sr*sr;
	float h = b*b - c;
	if (h<0.0) return -1.0;
	h = sqrt(h);
	return -b - h;
}

bool trace(vec3 ro, vec3 rd, out vec3 p, out vec3 n, out vec3 c, out vec3 e) {
	float tmin = -1;
	for (int i = 0; i < spheres.length(); ++i) {
		Sphere s = spheres[i];
		float t = isect_sphere(ro, rd, s.p, s.r);
		if (t > -1 && (tmin == -1 || t < tmin)) {
			tmin = t;
			p = ro + rd * t;
			n = normalize(p - s.p);
			c = s.c;
			e = s.e;
		}
	}
	return tmin > -1;
}
float shadow(vec3 ro, vec3 rd, float max) {
	for (int i = 0; i < spheres.length(); ++i) {
		Sphere s = spheres[i];
		float t = isect_sphere(ro, rd, s.p, s.r);
		if (t > -1 && t < max - FLT_EPSILON) {
			return 0.0;
		}
	}
	return 1.0;
}

vec3 light(vec3 p, vec3 n) {
	vec3 l = vec3(0, 100, 0);
	vec3 pl = l - p;
	vec3 lp = p - l;
	vec3 ld = normalize(pl);
	vec3 pd = normalize(lp);
	float ndl = max(0.0, dot(n, ld));
	float sh = shadow(l, pd, length(lp));
	return vec3(1.0) *ndl;// *sh;
}

vec3 render(vec3 ro, vec3 rd) {
	vec3 tcol = vec3(0);
	vec3 fcol = vec3(1);

	for (int i = 0; i < num_bounces; ++i) {
		vec3 p;
		vec3 n;
		vec3 c;
		vec3 e;
		if (trace(ro, rd, p, n, c, e)) {
			fcol *= c + e;
			//if (i != 0)
			{
				tcol += fcol * e;
			}

			ro = p;
			//rd = reflect(rd, n);
			//rd = n;
			rd = cosine_sample(n);
			ro += rd * 0.1;
		}
	}

	return tcol;
}

void main() {
	random_init2f(pixel);
	float aspect = resolution.x / resolution.y;
	vec3 col = vec3(0);

	for (int i = 0; i < num_rays; ++i) {
		vec2 p = pixel * vec2(aspect, 1) + snorm(random2f()) / resolution;

		vec3 ro = vec3(0, 0, -5);
		vec3 uu = vec3(1, 0, 0);
		vec3 vv = vec3(0, 1, 0);
		vec3 ww = vec3(0, 0, 1);

		vec3 er = normalize(vec3(p.xy, fov));
		vec3 rd = er.x*uu + er.y*vv + er.z*ww;

		vec3 go = blurAmount*vec3(snorm(random2f()), 0.0);
		vec3 gd = normalize(er*focusDistance - go);
		ro += go.x*uu + go.y*vv;
		rd += gd.x*uu + gd.y*vv;
		rd = normalize(rd);

		col += render(ro, rd);
	}
	col /= num_rays;
	col = pow(col, vec3(1/2.2));
	outColor = vec4(col, 1);
}
);

Raymarch::Raymarch() {
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	lsgl::compileshader(vertex_shader, vertex_src, lsgl_countof(vertex_src));
	
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	lsgl::compileshader(fragment_shader, fragment_src, lsgl_countof(fragment_src));

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	lsgl::linkprogram(program);

	glUseProgram(program);
	uniforms.random_seed = glGetUniformLocation(program, "random_seed");
	uniforms.resolution = glGetUniformLocation(program, "resolution");
	glUseProgram(0);
}

void Raymarch::Begin() {
	glUseProgram(program);
}

void Raymarch::Draw(int width, int height) {
	glUniform4ui(uniforms.random_seed, rand(), rand(), rand(), rand());
	glUniform2i(uniforms.resolution, width, height);
}

void Raymarch::End() {
	glUseProgram(0);
}