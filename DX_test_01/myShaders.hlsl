struct v2f
{
	float4 position : SV_POSITION;
	float4 color :COLOR;
};

v2f VShader(float4 position: POSITION, float4 color : COLOR)
{
	v2f output;

	output.position = position;
	output.color = color;

	return output;
}

float4 PShader(float4 position:SV_POSITION, float4 color : COLOR) :SV_TARGET
{
	return color;
}