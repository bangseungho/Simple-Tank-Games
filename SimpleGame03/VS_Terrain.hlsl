#define FRAME_BUFFER_WIDTH		800.f
#define FRAME_BUFFER_HEIGHT		600.f

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
struct VS_INPUT
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT VS_Terrain(VS_INPUT input)
{
    VS_OUTPUT output;
    output.position = mul(mul(mul(float4(input.position, 1.f), world), view), projection);
    output.color = input.color;

    return output;
}