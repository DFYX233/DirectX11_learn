struct v2f
{
	float4 position : SV_POSITON;
	float4 color :COLOR;
};

v2f VShader(float4 position: POSITON, float4 color : COLOR)
{
	v2f output;

	output.positon = positon;
	output.color = color;
}

float4 PShader(float4 positon:SV_POSITION, float4 color : COLOR) :SV_TARGET
{
	return color;
}