#include "Cube.hlsli"



// 像素着色器
float4 PS(VertexOut pIn) : SV_Target
{
   // return (1.0f,1.0f,1.0f,1.0f);
    return pIn.color;
}
