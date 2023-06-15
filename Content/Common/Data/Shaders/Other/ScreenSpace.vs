struct VS_OUT
{
	float4 Position : POSITION;
	float2 UV : TEXCOORD0;
};

VS_OUT main(float2 Position : POSITION)
{
	VS_OUT Out;
	Out.Position = float4(Position, 0.0f, 1.0f);
	Out.UV = Position.xy * float2(0.5f, -0.5f) + 0.5f;
	return Out;
}                                   