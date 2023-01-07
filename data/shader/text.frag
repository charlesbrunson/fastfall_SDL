

precision mediump float;
uniform sampler2D texture0;

in vec4 v_color;
in vec2 texCoord;

out vec4 FragColor;

void main()
{
	FragColor = texture(texture0, texCoord) * v_color;
}
