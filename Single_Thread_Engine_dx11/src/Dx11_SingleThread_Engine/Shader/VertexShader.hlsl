struct VSOut
{
	float4 color : Color;
	float4 pos : SV_Position;
};

cbuffer CBuf
{
	matrix transform;
};

float4 main(float3 pos : Position) : SV_Position//, float4 color : Color)
{
	//VSOut vso;
	//vso.pos = mul(float4(pos, 1.0f), transform);
	//vso.color = color;

	return float4(pos, 1.0f); // mul(float4(pos, 1.0f), transform);
}