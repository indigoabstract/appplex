#version 330 core
out vec4 FragColor;

/* Material definition for this geometry */
layout(std140) uniform Material
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 alpha;
    vec4 shininess;
}u_Material;

void main()
{
    FragColor = vec4(0.0, 1.0, 0.0, u_Material.alpha.a);
}
