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
[numthreads(2, 1, 1)]
void MSMain(
	in uint3 groupID : SV_GroupID,
	in uint3 threadInGroup : SV_GroupThreadID,
	out vertices MSvert verts[8],
	out indices uint3 idx[12]) // Three indices per primitive
{
	const uint numVertices = 8;
	const uint numPrimitives = 12;
	SetMeshOutputCounts(numVertices, numPrimitives);

	verts[0].pos = TransformPosition(float4(-0.5f, -0.5f, -0.5f, 1.0f));
	verts[0].color = float3(0.0f, 0.0f, 0.0f);

	verts[1].pos = TransformPosition(float4(-0.5f, -0.5f, 0.5f, 1.0f));
	verts[1].color = float3(0.0f, 0.0f, 1.0f);

	verts[2].pos = TransformPosition(float4(-0.5f, 0.5f, -0.5f, 1.0f));
	verts[2].color = float3(0.0f, 1.0f, 0.0f);

	verts[3].pos = TransformPosition(float4(-0.5f, 0.5f, 0.5f, 1.0f));
	verts[3].color = float3(0.0f, 1.0f, 1.0f);

	verts[4].pos = TransformPosition(float4(0.5f, -0.5f, -0.5f, 1.0f));
	verts[4].color = float3(1.0f, 0.0f, 0.0f);

	verts[5].pos = TransformPosition(float4(0.5f, -0.5f, 0.5f, 1.0f));
	verts[5].color = float3(1.0f, 0.0f, 1.0f);

	verts[6].pos = TransformPosition(float4(0.5f, 0.5f, -0.5f, 1.0f));
	verts[6].color = float3(1.0f, 1.0f, 0.0f);

	verts[7].pos = TransformPosition(float4(0.5f, 0.5f, 0.5f, 1.0f));
	verts[7].color = float3(1.0f, 1.0f, 1.0f);

	// Although we're running on multiple threads, the threads share
	// the same output arrays (e.g., of indices).
	if (threadInGroup.x == 0)
	{

		idx[0] = uint3(0, 2, 1);
		idx[1] = uint3(1, 2, 3);
		idx[2] = uint3(4, 5, 6);
		idx[3] = uint3(5, 7, 6);
		idx[4] = uint3(0, 1, 5);
		idx[5] = uint3(0, 5, 4);
	}	
	else
	{

		idx[6] = uint3(2, 6, 7);
		idx[7] = uint3(2, 7, 3);
		idx[8] = uint3(0, 4, 6);
		idx[9] = uint3(0, 6, 2);
		idx[10] = uint3(1, 3, 7);
		idx[11] = uint3(1, 7, 5);
	}	
}