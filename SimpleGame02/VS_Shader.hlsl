#define FRAME_BUFFER_WIDTH		800.f
#define FRAME_BUFFER_HEIGHT		600.f

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

struct VS_INPUT
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
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


VS_OUTPUT VS_Diffused(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float deathTime = timeAfterDeath;
    
    float3 newPosition;
    newPosition.xz = input.normal.xz * 1.5f * deathTime;
    newPosition.y = input.normal.y * 3.f * deathTime - deathTime * deathTime * 0.5;
    
    float t = input.position.y + newPosition.y;
    
    newPosition.x = clamp(newPosition.x, -10.f, 10.f);
    newPosition.z = clamp(newPosition.z, -10.f, 10.f);
    
    output.positionW = mul(float4(input.position + newPosition, 1.0f), world).xyz;
    
    if(output.positionW.y < 0)
        output.positionW = 0.f;
    
    output.positionH = mul(mul(float4(output.positionW, 1.0f), view), projection);
    output.normalW = mul(float4(input.normal, 0.0f), world).xyz;
    output.normal = input.normal;
    output.uv = input.uv;
    output.color = lerp(float4(normalize(output.positionW), 1.0f), float4(normalize(output.normalW), 1.0f), 0.5f);
    
    return (output);
}