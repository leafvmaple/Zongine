//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

cbuffer cbPerObject
{
	float4x4 gWorldViewProj; 
};

struct VertexInput
{
	float3 PosL  : POSITION;
    float3 Normal: NORMAL;
    float4 Color : COLOR;
    float2 Texcoord : TEXCOORD;
    float4 BoneWeights : BONEWEIGHTS;
    float4 BoneIndeces : BONEINDICES;
    float4 Tangent :  TANGENT;
};

struct VertexOutput
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOutput VS(VertexInput vin)
{
	VertexOutput vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOutput pin) : SV_Target
{
    return pin.Color;
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
