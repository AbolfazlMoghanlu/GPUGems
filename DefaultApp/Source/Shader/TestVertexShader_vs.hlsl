cbuffer Matrices : register(b0)
{
	float4x4 TransformMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float3 CameraPosition;
	float Pad[13];
};

cbuffer PSConstantBufferLayout : register(b1)
{
	float3 ColorOverlay;
	float Pad1[61];
}

struct VSOut
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 UVs : UV;
};

VSOut main(float3 inpos : POSITION, float3 incolor : Color, float2 inuv : UV)
{
	VSOut Out;

	Out.pos = float4(inpos, 1);
	Out.pos = mul(TransformMatrix, Out.pos);
	Out.pos = mul(ViewMatrix, Out.pos);
	Out.pos = mul(ProjectionMatrix, Out.pos);

	Out.color = float4(incolor, 1);
	Out.UVs = inuv;

	return Out;
}