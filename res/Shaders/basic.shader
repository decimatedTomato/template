#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

void main()
{
   gl_Position = position;
};

#shader fragment
#version 330 core

in vec4 gl_FragCoord;
out vec4 fragColor;
// layout(location = 0) out vec4 color;

uniform vec2 u_resolution;
uniform float u_time;

void main()
{
   // Normalized pixel coordinates (from 0 to 1)
   vec2 coord = gl_FragCoord.xy / u_resolution.xy;

   // Time varying pixel color
   vec3 col = 0.5 + 0.5*cos(u_time + coord.xyx + vec3(0,2,4));

   // Output to screen
   fragColor = vec4(col,1.0);
};