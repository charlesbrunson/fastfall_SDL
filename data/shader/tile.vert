
uniform mat3 model;
uniform mat3 view;
uniform sampler2D texture0;

uniform uint columns;

layout (location = 0) in uint aTileId;

out vec2 texCoord;
out float valid;

void main()
{
	const float TILESIZE = 16.0;

	const uint Y_MASK  = (63u << 6);
	const uint X_MASK  = (63u);
	const uint XY_MASK = (X_MASK | Y_MASK);
	const uint INVALID = 4095u;

	// non-empty tile
	valid = float((aTileId & XY_MASK) != INVALID);

	uint pad_top 	= (aTileId >> 15) & 1u;
	uint pad_right 	= (aTileId >> 14) & 1u;
	uint pad_bot 	= (aTileId >> 13) & 1u;
	uint pad_left 	= (aTileId >> 12) & 1u;

	uvec2 tile_id = uvec2(
		aTileId & X_MASK,
		(aTileId & Y_MASK) >> 6
	);

	vec2 tileset_size = vec2(textureSize(texture0, 0)) / TILESIZE;

	// +1 for right/bot, -1 for left/top
	float horz_side = float(gl_VertexID & 1) * 2.0 - 1.0;
	float vert_side = float((gl_VertexID & 2) >> 1) * 2.0 - 1.0;

	vec2 t_offset = vec2(
		horz_side,
		vert_side
	) / -16384.0;

	vec2 p_offset = vec2(
		(0.5 + horz_side * 0.5),
		(0.5 + vert_side * 0.5)
	);

	// add padding to position offset
	p_offset.x = p_offset.x
		+ (-1.0 * float(pad_left)  * float((gl_VertexID + 1) & 1))
		+ ( 1.0 * float(pad_right) * float((gl_VertexID)     & 1));

	p_offset.y = p_offset.y
		+ (-1.0 * float(pad_top) * float((((gl_VertexID & 2) >> 1) + 1) & 1) )
		+ ( 1.0 * float(pad_bot) * float(( (gl_VertexID & 2) >> 1)));

	// position of tile based on instance id + position offset
	vec2 position = p_offset + vec2(
		float(uint(gl_InstanceID) % columns),
		float(uint(gl_InstanceID) / columns)
	);

	// apply transform
	gl_Position = vec4( view * model * vec3(position * TILESIZE, 1.0), 1.0);

	// calc texture coords
	const uint tileset_columns = 64u;
	texCoord    = t_offset + vec2(
		(float(tile_id.x) + p_offset.x) / tileset_size.x,
		(float(tile_id.y) + p_offset.y) / tileset_size.y
	);
}

