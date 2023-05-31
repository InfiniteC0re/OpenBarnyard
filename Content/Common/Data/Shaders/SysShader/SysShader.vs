struct VS_IN {
	float3 ObjPos : POSITION;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
};

struct VS_OUT {
	float4 ProjPos : SV_POSITION;
	float4 Color : COLOR;
	float2 UV : TEXCOORD0;
};

float4x4 matWVP : register(c0);
float4x4 matWV : register(c4);
float3x3 matNorm : register(c8);
float4x4 texMtx0 : register(c12);
float4x4 texMtx1 : register(c16);
float4x4 texMtx2 : register(c20);
float4x4 texMtx3 : register(c24);
float4x4 texMtx6 : register(c28);
float4x4 texMtx7 : register(c32);
float4 fogStartEnd : register(c36);

VS_OUT main(VS_IN In) {
	VS_OUT Out;
	Out.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0));
	Out.Color = In.Color;
	Out.UV = In.UV;
	return Out;
}