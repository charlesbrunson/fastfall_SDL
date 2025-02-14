
precision mediump float;
uniform sampler2D texture0;
in vec2 texCoord;
in float valid;
out vec4 FragColor;

void main()
{
    FragColor = valid * texture(texture0, texCoord);
}
