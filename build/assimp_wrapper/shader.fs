#version 130
//#version 330

uniform sampler2D sampler0;

in vec2 tcoord;
out vec4 colorOut;

void main()
{
  colorOut = texture(sampler0, tcoord);
  //colorOut = vec4(0.0, 0.0, 1.0, 1.0);
}
