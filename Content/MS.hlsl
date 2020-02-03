// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

struct MSvert
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
};

float4 TransformPosition(float4 v)
{
	v = mul(v, model);
	v = mul(v, view);
	v = mul(v, projection);
	return v;
}

[outputtopology("triangle")]
[numthreads(12, 1, 1)]
void MSMain(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[8],
	out indices uint3 idx[12]) // Three indices per primitive
{
	// Although we're running on multiple threads, the threads share
	// the same output arrays (e.g., of indices).
	const uint numVertices = 8;
	const uint numPrimitives = 12;
	SetMeshOutputCounts(numVertices, numPrimitives);

	const float4 allVertices[] = {
		float4(-0.5f, -0.5f, -0.5f, 1.0f),
		float4(-0.5f, -0.5f, 0.5f, 1.0f),
		float4(-0.5f, 0.5f, -0.5f, 1.0f),
		float4(-0.5f, 0.5f, 0.5f, 1.0f),
		float4(0.5f, -0.5f, -0.5f, 1.0f),
		float4(0.5f, -0.5f, 0.5f, 1.0f),
		float4(0.5f, 0.5f, -0.5f, 1.0f),
		float4(0.5f, 0.5f, 0.5f, 1.0f)
	};

	const float3 allColors[] = {
		float3(0.0f, 0.0f, 0.0f),
		float3(0.0f, 0.0f, 1.0f),
		float3(0.0f, 1.0f, 0.0f),
		float3(0.0f, 1.0f, 1.0f),
		float3(1.0f, 0.0f, 0.0f),
		float3(1.0f, 0.0f, 1.0f),
		float3(1.0f, 1.0f, 0.0f),
		float3(1.0f, 1.0f, 1.0f)
	};

	const uint3 allIndices[] = {
		uint3(0, 2, 1),
		uint3(1, 2, 3),
		uint3(4, 5, 6),
		uint3(5, 7, 6),
		uint3(0, 1, 5),
		uint3(0, 5, 4),
		uint3(2, 6, 7),
		uint3(2, 7, 3),
		uint3(0, 4, 6),
		uint3(0, 6, 2),
		uint3(1, 3, 7),
		uint3(1, 7, 5)
	};

	uint tid = threadInGroup.x;

	if (tid < numVertices)
	{
		verts[tid].pos = TransformPosition(allVertices[tid]);
		verts[tid].color = allColors[tid];
	}

	idx[tid] = allIndices[tid];
}