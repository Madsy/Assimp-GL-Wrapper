#version 130
//#version 330

#define MAX_BONES_PER_VERTEX 4
#define MAX_BONES_PER_MESH 32

uniform mat4 projection;
uniform mat4 sc_modelview;
uniform mat4 sc_camera;
uniform mat4 sc_bones[MAX_BONES_PER_MESH];

in vec3 sc_vertex;
in vec3 sc_normal;
in vec3 sc_tangent;
in vec3 sc_bitangent;
in vec3 sc_tcoord0;
in vec3 sc_tcoord1;
in vec3 sc_tcoord2;
in vec3 sc_tcoord3;
in uvec4 sc_index;
in vec4 sc_weight; 

out vec2 tcoord;

vec4 animateBone(vec4 p)
{
  vec4 vOut;

  uint idx[4] = uint[4](sc_index.x, sc_index.y, sc_index.z, sc_index.w);
  float weight[4] = float[4](sc_weight.x, sc_weight.y, sc_weight.z, sc_weight.w);

  vOut  = (sc_bones[idx[0]] * p) * weight[0];
  vOut += (sc_bones[idx[1]] * p) * weight[1];
  vOut += (sc_bones[idx[2]] * p) * weight[2];
  vOut += (sc_bones[idx[3]] * p) * weight[3];
  return vOut;
}

void main()
{
  tcoord = sc_tcoord0.xy;
  vec4 v = animateBone(vec4(sc_vertex, 1.0));
  gl_Position = projection * sc_camera * v;
  //vec4 v = vec4(sc_vertex, 1.0);
  //gl_Position = projection * sc_camera * v;
}
