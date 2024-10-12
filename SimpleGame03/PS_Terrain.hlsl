#define FRAME_BUFFER_WIDTH		800.f
#define FRAME_BUFFER_HEIGHT		600.f

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

float4 PS_Terrain(VS_OUTPUT input) : SV_TARGET
{
    return input.color;
}

// smoothstep(x, y, value) : value의 값이 x보다 작으면 0의 값을 y보다 크면 1의 값을 반환하며 증간 값은 보간한다.
