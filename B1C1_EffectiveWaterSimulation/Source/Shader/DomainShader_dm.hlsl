cbuffer Matrices : register(b0)
{
	float4x4 TransformMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
	float3 CameraPosition;
	float Time;
	float Pad[12];
};

cbuffer PSConstantBufferLayout : register(b1)
{
	float3 ColorOverlay;
	float TesselationAmount;
	float Height;
	float TesselationOffset;
	float TesselationLength;
	float WaveLength;

	float3 Direction;
	float Amplitude;

	float Speed;
	float Pad1[51];
}


struct Patch
{
	float edgeTessFactor[3] : SV_TessFactor;
	float inTessFactor : SV_InsideTessFactor;
};

struct HS_OUTPUT
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 UVs : UV;
};

[domain("tri")]
HS_OUTPUT main(Patch input,
	float3 UVW : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, 3> patch)
{
	HS_OUTPUT Output;

	Output.pos = patch[0].pos * UVW.x + patch[1].pos * UVW.y + patch[2].pos * UVW.z;
	Output.color = patch[0].color * UVW.x + patch[1].color * UVW.y + patch[2].color * UVW.z;
	Output.UVs = patch[0].UVs * UVW.x + patch[1].UVs * UVW.y + patch[2].UVs * UVW.z;

	Output.pos = mul(TransformMatrix, Output.pos);

	float W = Amplitude * sin(dot(Direction.xy, Output.pos.xz) * 2 / WaveLength + Time * Speed * 2 / WaveLength);

	Output.pos += float4(0, W, 0, 0);

	Output.pos = mul(ViewMatrix, Output.pos);
	Output.pos = mul(ProjectionMatrix, Output.pos);
	
	return Output;
}