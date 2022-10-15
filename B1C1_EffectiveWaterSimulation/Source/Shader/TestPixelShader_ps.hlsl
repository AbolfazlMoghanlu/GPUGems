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

SamplerState LinearSampler: register(s0);

Texture2D<float4> UVCheckerTexture : register(t0);

float4 main(float4 pos : SV_Position, float4 color : Color, float2 UVs : UV) : SV_TARGET
{
	float4 CheckerBoard = UVCheckerTexture.Sample(LinearSampler, UVs);

	return color * float4(ColorOverlay, 1.0f) * CheckerBoard;
}