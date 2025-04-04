#include "Basic.hlsli"

// 顶点着色器(3D)
VertexPosHWNormalTex VS(VertexPosNormalTex vIn)
{
    VertexPosHWNormalTex vOut;
    matrix viewProj = mul(g_View, g_Proj);
    float4 posW = mul(float4(vIn.posL, 1.0f), g_World);

    vOut.posH = mul(posW, viewProj);
    vOut.posW = posW.xyz;
    vOut.normalW = mul(vIn.normalL, (float3x3) g_WorldInvTranspose);
    // 平移到旋转中心点
    float2 offsetTexCoord = vIn.tex - 0.5;

    // 应用旋转矩阵
    float2 rotatedTexCoord;
    rotatedTexCoord.x = g_TexCoordTransform[0][0] * offsetTexCoord.x + g_TexCoordTransform[0][1] * offsetTexCoord.y;
    rotatedTexCoord.y = g_TexCoordTransform[1][0] * offsetTexCoord.x + g_TexCoordTransform[1][1] * offsetTexCoord.y;

    // 移回原来的位置
    vOut.tex = rotatedTexCoord + 0.5;
    return vOut;
}
