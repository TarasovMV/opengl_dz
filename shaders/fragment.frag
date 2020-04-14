#version 330 core
in vec3 vert_color;

out vec4 color;

void main()
{
  color = vec4(vert_color, 1.0f);
  //color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}