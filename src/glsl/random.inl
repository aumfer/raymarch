GLSL(
// http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}
// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}

uniform uvec4 random_seed;
uvec4 seed = random_seed;
void random_init1ui(uint x) {
	seed ^= uvec4(x, ~x, -x, -~x);
}
void random_init2ui(uvec2 x) {
	seed ^= uvec4(x.x, x.y, ~x.x, ~x.y);
}
void random_init2f(vec2 x) {
	seed ^= uvec4(floatBitsToUint(x.x), floatBitsToUint(x.y), floatBitsToUint (-x.x), floatBitsToUint (-x.y));
}
float random1f() {
	return floatConstruct(hash(seed++));
}
vec2 random2f() {
	return vec2(random1f(), random1f());
}
vec3 random3f() {
	return vec3(random1f(), random1f(), random1f());
}
float normal1f() {
	return sqrt(random1f()) * cos(random1f() * M_PI * 2);
}
vec2 normal2f() {
	return vec2(normal1f(), normal1f());
}

/* https://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/ */
vec3 cosine_sample(vec3 n) {
	vec2 Xi = random2f();

	float theta = acos(sqrt(1.0 - Xi.x));
	float phi = 2.0 * M_PI * Xi.y;

	float xs = sin(theta) * cos(phi);
	float ys = cos(theta);
	float zs = sin(theta) * sin(phi);

	vec3 y = n;
	vec3 h = y;
	if (abs(h.x) <= abs(h.y) && abs(h.x) <= abs(h.z)) {
		h.x = 1.0;
	} else if (abs(h.y) <= abs(h.x) && abs(h.y) <= abs(h.z)) {
		h.y = 1.0;
	} else {
		h.z = 1.0;
	}

	vec3 x = normalize(cross(h, y));
	vec3 z = normalize(cross(x, y));

	vec3 direction = normalize(xs * x + ys * y + zs * z);
	return direction;
}

float rand(vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
vec2 hammersley2d(uint i, uint N) {
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}
)