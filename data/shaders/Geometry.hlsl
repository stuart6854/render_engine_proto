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
    [[vk::location(0)]] float3 PositionWS : POSITION0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
    [[vk::location(2)]] float3 NormalWS : NORMAL0;
    [[vk::location(3)]] float3 TangentWS : TANGENT0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    output.FragPos = mul(consts.viewProjMatrix, mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)));

    output.PositionWS = mul(consts.modelMatrix, float4(input.Position.xyz, 1.0)).xyz;
    output.TexCoord = input.TexCoord;
    output.NormalWS = normalize(input.Normal);
    output.TangentWS = normalize(input.Tangent);
    return output;
}

struct PSInput
{
    [[vk::location(0)]] float3 PositionWS : POSITION0;
    [[vk::location(1)]] float2 TexCoord : TEXCOORD0;
    [[vk::location(2)]] float3 NormalWS : NORMAL0;
    [[vk::location(3)]] float3 TangentWS : TANGENT0;
};

struct PSOutput
{
    float4 FragColor : SV_TARGET0;
};

static float AMBIENT_STRENGTH = 0.1;
static float3 AMBIENT_COLOR = float3(1.0, 1.0, 1.0);

static float3 LIGHT_POS = float3(1.2, 1.0, 2.0);
static float3 LIGHT_COLOR = float3(1.0, 1.0, 1.0);

PSOutput PSMain(PSInput input) 
{
    PSOutput output;

    float3 objectColor = float3(1.0, 1.0, 1.0);
    float3 objectNorm = normalize(input.NormalWS);

    float3 ambientColor = AMBIENT_COLOR * AMBIENT_STRENGTH;

    float3 lightDir = normalize(LIGHT_POS - input.PositionWS);
    float diff = max(dot(input.NormalWS, lightDir), 0.0);
    float3 diffuseColor = diff * LIGHT_COLOR;

    float3 finalColor = objectColor * (ambientColor + diffuseColor);
    output.FragColor = float4(finalColor, 1.0);

    return output;
}