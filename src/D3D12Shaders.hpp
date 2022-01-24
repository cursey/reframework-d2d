#pragma once

const char* D3D12_VERT_SHADER = R"(
cbuffer vert_buffer : register(b0) {
    float4x4 mvp;
};

struct VS_INPUT {
    float2 pos : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(mvp, float4(input.pos.xy, 0.0f, 1.0f));
    output.color = input.color;
    output.uv = input.uv;
    return output;
}
)";

const char* D3D12_PIX_SHADER = R"(
struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

float4 main(PS_INPUT input) : SV_TARGET {
    return input.color * texture0.Sample(sampler0, input.uv);
}
)";
