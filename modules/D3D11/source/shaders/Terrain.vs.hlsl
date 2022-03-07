#pragma pack_matrix(row_major)

cbuffer Transformation
{
	float4x4 transform;
};

static float2 vertices[] =
{
	float2(-1.f, -1.f),
	float2(-1.f,  1.f),
	float2( 1.f,  1.f),
	float2( 1.f, -1.f),
};

struct Input
{
	uint vertexIndex : SV_VertexID;
};

float4 vertexShaderMain(Input input) : SV_POSITION
{
	return mul(float4(vertices[input.vertexIndex], 0.f, 1.f), transform);
}