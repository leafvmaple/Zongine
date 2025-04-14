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
    float3 WorldNormal;
};

struct ParamsMainVertexNode
{
    float3  WorldPositionOffset;
};

struct ParamsMainPixelNode
{
    float3  Albedo;
    float   Fresnel;
    float3  Normal;
    float3  Emissive;
    float   Opacity; // 透明度
    float3  Ambient;
};

#endif