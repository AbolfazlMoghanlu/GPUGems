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

struct VSOut
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 UVs : UV;
};


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

Patch ConstantHS(InputPatch<VSOut, 3> patch, uint pI:SV_PrimitiveID)
{
	Patch p;

	float3 Pos0 = mul(TransformMatrix, patch[0].pos).xyz;
	float3 Pos1 = mul(TransformMatrix, patch[1].pos).xyz;
	float3 Pos2 = mul(TransformMatrix, patch[2].pos).xyz;

	float CameraToPatch0 = length(Pos0 - CameraPosition);
	float CameraToPatch1 = length(Pos1 - CameraPosition);
	float CameraToPatch2 = length(Pos2 - CameraPosition);

	float DistanceFactor0 = saturate((CameraToPatch0 - TesselationOffset) / TesselationLength);
	float DistanceFactor1 = saturate((CameraToPatch1 - TesselationOffset) / TesselationLength);
	float DistanceFactor2 = saturate((CameraToPatch2 - TesselationOffset) / TesselationLength);

	p.edgeTessFactor[0] = lerp(TesselationAmount, 1, DistanceFactor0);
	p.edgeTessFactor[1] = lerp(TesselationAmount, 1, DistanceFactor1);
	p.edgeTessFactor[2] = lerp(TesselationAmount, 1, DistanceFactor2);

	p.inTessFactor = (p.edgeTessFactor[0] + p.edgeTessFactor[1] + p.edgeTessFactor[2]) / 3;

	return p;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HS_OUTPUT main(InputPatch<VSOut, 3> patch, uint i : SV_OutputControlPointID, uint pI : SV_PrimitiveID)
{
	HS_OUTPUT output;
	output.pos = patch[i].pos;
	output.color = patch[i].color;
	output.UVs = patch[i].UVs;

	return output;
}