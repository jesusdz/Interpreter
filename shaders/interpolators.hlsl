
struct VertexOutput
{
	float4 position : SV_Position;
	float4 positionWs : POSITION0;
	float3 normalWs : NORMAL0;
	float2 texCoord : TEXCOORD0;
};

#define PixelInput VertexOutput

