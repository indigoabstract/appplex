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
    //FragColor = vec4(1.0, 0.0, 0.0, alpha.a);
    FragColor = vec4(1.0);
    //FragColor = vec4(u_Material.shininess.rgb, u_Material.alpha.a);
}
