#ifndef __NODE_INTERFACE_H_
#define __NODE_INTERFACE_H_

struct MaterialVertexParameters
{
    float4 VertexPosition;
    float4 VertexColor;

	float2 TexCoords;

	float4x4 MatrixWorld;
};

struct MaterialPixelParameters
{
    float2 TexCoords;
};

struct ParamsMainPixelNode
{
    float3  Albedo;
    float3  Emissive;
    float   Opacity; // 透明度
    float3  Ambient;
};

#endif