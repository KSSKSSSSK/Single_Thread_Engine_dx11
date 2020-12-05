cbuffer CBuf
{
	float4 face_colors[6];
};

float4 main() : SV_TARGET
{
	return float4(255,255,0, 255);
}