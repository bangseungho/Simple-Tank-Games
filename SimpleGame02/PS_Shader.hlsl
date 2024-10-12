cbuffer cbFrameworkInfo : register(b0)
{
    float currentTime;
    float elapsedTime;
    float2 cursorPos;
};

cbuffer cbGameObjectInfo : register(b1)
{
    matrix world : packoffset(c0);
    float timeAfterDeath : packoffset(c4);
};

cbuffer cbCameraInfo : register(b2)
{
    matrix view : packoffset(c0);
    matrix projection : packoffset(c4);
    float3 cameraPos : packoffset(c8);
};

struct VS_OUTPUT
{
    float4 positionH : SV_POSITION;
    float3 positionW : POSITION;
    float3 normal : NORMAL0;
    float3 normalW : NORMAL1;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

static float3 gf3LightDirection = float3(1.4142f, 1.4142f * 0.5f, 1.4142f * 0.5f);
static float3 gf3LightColor = float3(0.65f, 0.55f, 0.65f);
static float3 gf3SpecularColor = float3(0.35f, 0.75f, 0.75f);

static float gfSpecular = 2.0f;
static float gfGlossiness = 1.8f;

float4 PS_Diffused(VS_OUTPUT input) : SV_TARGET
{
    float4 cColor = input.color;

    float3 f3Normal = normalize(input.normalW);
    float fDiffused = max(0.0f, dot(f3Normal, normalize(gf3LightDirection)));
    cColor.rgb = input.color.rgb + gf3LightColor * fDiffused;

    float3 f3ToCamera = normalize(cameraPos - input.positionW);
    float3 f3Half = normalize(gf3LightDirection + f3ToCamera);
    float fHalf = max(0.0f, dot(f3Normal, f3Half));
    float fSpecular = pow(fHalf, gfSpecular * 128.0f) * gfGlossiness;
    cColor.rgb += gf3SpecularColor * fSpecular + ((f3Normal / 2.0f) + 0.5f) * 0.3f;

    return (cColor);
}
