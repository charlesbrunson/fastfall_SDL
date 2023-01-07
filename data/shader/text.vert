
uniform mat3 model;
uniform mat3 view;
uniform sampler2D texture0;

uniform vec2 char_size;

out vec4 v_color;
out vec2 texCoord;

layout (location = 0) in uint character;
layout (location = 1) in vec2 offset;
layout (location = 2) in vec4 color;

void main()
{
	const uint CHARS_PER_COLUMN = 16u;

	float horz_side = float(gl_VertexID & 1) * 2.0 - 1.0;
	float vert_side = float((gl_VertexID & 2) >> 1) * 2.0 - 1.0;

	vec2 p_offset = vec2(
		(0.5 + horz_side * 0.5),
		(0.5 + vert_side * 0.5)
	);

	vec2 pos = vec2(
		offset + (p_offset * char_size)
	);

	vec2 tpos = (vec2(
		float(character % CHARS_PER_COLUMN),
		float(character / CHARS_PER_COLUMN)
	) + p_offset) * char_size;

	gl_Position = vec4( view * model * vec3(pos, 1.0), 1.0);

	v_color = color;

	texCoord = tpos / vec2(textureSize(texture0, 0));
}
