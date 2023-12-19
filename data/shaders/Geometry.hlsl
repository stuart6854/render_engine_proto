struct VSInput
{
	[[vk::location(0)]] float3 Position : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PushConsts
{
	float4x4 viewProjMatrix;
	float4x4 modelMatrix;
};
[[vk::push_constant]] PushConsts consts;

struct VSOutput
{
	float4 FragPos : SV_POSITION;
	[[vk::location(0)]] float3 WorldPos : POSITION0;
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

VSOutput VSMain(VSInput input)
{
	VSOutput output;

	output.FragPos = mul(consts.viewProjMatrix, mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)));

	output.WorldPos = mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)).xyz;
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(input.Normal);
	output.Tangent = normalize(input.Tangent);
	return output;
}

struct PSInput
{
	[[vk::location(1)]] float2 TexCoord : TEXCOORD0;
	[[vk::location(2)]] float3 Normal : NORMAL0;
	[[vk::location(3)]] float3 Tangent : TANGENT0;
};

struct PSOutput
{
	float4 FragColor : SV_TARGET0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput output;

	// output.Albedo = textureColor.Sample(samplerColor, input.UV);
	output.FragColor = float4(1, 1, 1, 1);

	return output;
}