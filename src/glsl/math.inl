GLSL(
	float unorm(float v) {
	return (v * 0.5) + 0.5;
}
vec2 unorm(vec2 v) {
	return vec2(unorm(v.x), unorm(v.y));
}
vec3 unorm(vec3 v) {
	return vec3(unorm(v.x), unorm(v.y), unorm(v.z));
}
float snorm(float v) {
	return v * 2 - 1;
}
vec2 snorm(vec2 v) {
	return vec2(snorm(v.x), snorm(v.y));
}
vec3 snorm(vec3 v) {
	return vec3(snorm(v.x), snorm(v.y), snorm(v.z));
}

/* http://aras-p.info/texts/CompactNormalStorage.html */
vec2 encode(vec3 n) {
	vec2 enc = normalize(n.xy) * (sqrt(-n.z*0.5 + 0.5));
	enc = enc*0.5 + 0.5;
	return enc;
}
vec3 decode(vec2 enc) {
	vec4 nn = vec4(enc, 0, 0)*vec4(2, 2, 0, 0) + vec4(-1, -1, 1, -1);
	float l = dot(nn.xyz, -nn.xyw);
	nn.z = l;
	nn.xy *= sqrt(l);
	return nn.xyz * 2 + vec3(0, 0, -1);
}
)