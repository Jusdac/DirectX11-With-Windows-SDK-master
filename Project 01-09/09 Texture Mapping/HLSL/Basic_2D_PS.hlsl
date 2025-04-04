#include "Basic.hlsli"

// 像素着色器(2D)
float4 PS(VertexPosHTex pIn) : SV_Target
{
    //float4 color1 = g_Tex.Sample(g_SamLinear,pIn.tex);
    //float4 color2 = g_Tex1.Sample(g_SamLinear, pIn.tex);
    return g_Tex.Sample(g_SamLinear, pIn.tex);
    //return color1 + color2;
    //return (0.0f, 0.0f, 0.0f, 0.0f);
}