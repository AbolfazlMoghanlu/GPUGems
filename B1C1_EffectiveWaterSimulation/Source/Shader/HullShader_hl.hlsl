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
	float TesselationAmount = 64;
	float Height = 2.0f;
	float TesselationOffset = 5.0f;
	float TesselationLength = 15.0f;
	float Pad1[57];
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

	p.edgeTessFactor[0] = TesselationAmount;
	p.edgeTessFactor[1] = TesselationAmount;
	p.edgeTessFactor[2] = TesselationAmount;
	p.inTessFactor = TesselationAmount;

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