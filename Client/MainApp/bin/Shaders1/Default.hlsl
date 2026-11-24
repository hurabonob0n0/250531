//***************************************************************************************
// Default.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
    float3 NormalL : NORMAL;
	float2 TexC    : TEXCOORD;
	float3 TangentU : TANGENT;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float4 ShadowPosH : POSITION0;
    float3 PosW    : POSITION1;
    float3 NormalW : NORMAL;
	float3 TangentW : TANGENT;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Fetch the material data.
	MaterialData matData = gMaterialData[gMaterialIndex];
	
    // Bend the tank track onto the road wheels (no-op for every other mesh).
    float3 posL = vin.PosL;
    posL.z += TrackSag(posL);

    // Transform to world space.
    float4 posW = mul(float4(posL, 1.0f), gWorld);
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);
	
	vout.TangentW = mul(vin.TangentU, (float3x3)gWorld);

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, matData.MatTransform).xy;

    // Generate projective tex-coords to project shadow map onto scene.
    vout.ShadowPosH = mul(posW, gShadowTransform);
	
    return vout;
}

//---------------------------------------------------------------------------------------
// Object tints: terrain play area (gObjPad0 == 1) and team color (gObjPad1, Mesh.cpp).
// They used to be "+0.05 / +0.1 on the final color", picked by eye when the shader worked in
// gamma space. In linear space a constant add swamps dark pixels (0.05 linear is ~0.25 on
// screen), and the tank and terrain turned completely blue. So they are albedo multipliers
// now: they scale with the lighting just like the surface color does.
// The border line (5 units wide at radius 300) stays a flat unlit blue.
//---------------------------------------------------------------------------------------
float3 ObjectAlbedoTint(float3 posW, out bool isBorderLine)
{
    float3 tint = float3(1.0f, 1.0f, 1.0f);
    isBorderLine = false;

    if (gObjPad0 == 1)
    {
        float dist = length(posW.xz);
        float radius = 300.f;
        if (dist >= radius - 5.f && dist <= radius)
            isBorderLine = true;
        if (dist < radius - 5.f)
            tint.b *= 1.2f;             // was litColor.b += 0.05 (gamma)
    }

    if (gObjPad1 == 1)
        tint.r *= 1.5f;                 // was litColor.r += 0.1
    else if (gObjPad1 == 2)
        tint.b *= 1.5f;                 // was litColor.b += 0.1
    else if (gObjPad1 == 3)
        tint *= 0.65f;                  // was litColor.rgb -= 0.1

    return tint;
}

#ifdef GBUFFER
//---------------------------------------------------------------------------------------
// Deferred geometry pass: same inputs as the forward PS below, but instead of lighting
// the pixel it stores what the lighting needs into the G-Buffer (see Common.hlsl).
// Lighting happens later, once per visible pixel, in DeferredLighting.hlsl.
//---------------------------------------------------------------------------------------
GBufferOut PS(VertexOut pin)
{
	MaterialData matData = gMaterialData[gMaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
	float4 diffuseTex = gTextureMaps[matData.DiffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
	diffuseAlbedo *= float4(SRGBToLinear(diffuseTex.rgb), diffuseTex.a);	// sRGB texture -> linear

#ifdef ALPHA_TEST
    clip(diffuseAlbedo.a - 0.1f);
#endif

    pin.NormalW = normalize(pin.NormalW);
	float4 normalMapSample = gTextureMaps[matData.NormalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);

    const float shininess = (1.0f - matData.Roughness) * normalMapSample.a;

    // Tints go into the albedo (see ObjectAlbedoTint). Only the border line uses emissive (unlit blue).
    bool isBorderLine;
    diffuseAlbedo.rgb *= ObjectAlbedoTint(pin.PosW, isBorderLine);
    float  flag = isBorderLine ? GBUFFER_FLAG_UNLIT : GBUFFER_FLAG_LIT;
    float3 emissive = isBorderLine ? float3(0.f, 0.f, 1.f) : float3(0.f, 0.f, 0.f);

    GBufferOut gout;
    gout.Albedo   = float4(diffuseAlbedo.rgb, shininess);
    gout.Normal   = float4(bumpedNormalW, 0.0f);
    gout.Material = float4(matData.FresnelR0, flag);
    gout.Emissive = float4(emissive, 0.0f);
    return gout;
}

#else

float4 PS(VertexOut pin) : SV_Target
{
	// Fetch the material data.
	MaterialData matData = gMaterialData[gMaterialIndex];
	float4 diffuseAlbedo = matData.DiffuseAlbedo;
	float3 fresnelR0 = matData.FresnelR0;
	float  roughness = matData.Roughness;
	uint diffuseMapIndex = matData.DiffuseMapIndex;
	uint normalMapIndex = matData.NormalMapIndex;
	
    // Dynamically look up the texture in the array.
    float4 diffuseTex = gTextureMaps[diffuseMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    diffuseAlbedo *= float4(SRGBToLinear(diffuseTex.rgb), diffuseTex.a);	// sRGB texture -> linear

    bool isBorderLine;
    diffuseAlbedo.rgb *= ObjectAlbedoTint(pin.PosW, isBorderLine);   // team / play area tint (linear, multiplicative)

#ifdef ALPHA_TEST
    // Discard pixel if texture alpha < 0.1.  We do this test as soon 
    // as possible in the shader so that we can potentially exit the
    // shader early, thereby skipping the rest of the shader code.
    clip(diffuseAlbedo.a - 0.1f);
#endif

	// Interpolating normal can unnormalize it, so renormalize it.
    pin.NormalW = normalize(pin.NormalW);
	
	float4 normalMapSample = gTextureMaps[normalMapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample.rgb, pin.NormalW, pin.TangentW);

    if (gDebugMode == 1) {
        bumpedNormalW = bumpedNormalW * 0.5f + 0.5f;
        return float4(bumpedNormalW, 1.f);
    }
    
	// Uncomment to turn off normal mapping.
    //bumpedNormalW = pin.NormalW;

    // Vector from point being lit to eye. 
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // Light terms.
    float4 ambient = gAmbientLight*diffuseAlbedo;

    // Only the first light casts a shadow.
    float3 shadowFactor = float3(1.0f, 1.0f, 1.0f);
    shadowFactor[0] = CalcShadowFactor(pin.ShadowPosH);
    if (gDebugMode == 2)
    {
        float s = shadowFactor[0];
        return float4(s, s, s, 1.f);
    }

    const float shininess = (1.0f - roughness) * normalMapSample.a;
    Material mat = { diffuseAlbedo, fresnelR0, shininess };
    float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        bumpedNormalW, toEyeW, shadowFactor);

    float4 litColor = ambient + directLight;

    // Forward point lights: every object pixel loops over every point light (count is 0 on the deferred paths)
    litColor.rgb += AccumulatePointLights(mat, pin.PosW, bumpedNormalW, toEyeW);

	// Add in specular reflections.
    float3 r = reflect(-toEyeW, bumpedNormalW);
    float4 reflectionColor = gCubeMap.Sample(gsamLinearWrap, r);
    reflectionColor.rgb = SRGBToLinear(reflectionColor.rgb);
    float3 fresnelFactor = SchlickFresnelRoughness(fresnelR0, bumpedNormalW, r, shininess);   // rough surfaces: no mirror at grazing angles
    litColor.rgb += shininess * fresnelFactor * reflectionColor.rgb;
	
    // Common convention to take alpha from diffuse albedo.
    litColor.a = diffuseAlbedo.a;
    
    if (isBorderLine)
        litColor.rgb = float3(0.f, 0.f, 1.f);
    
    
    return litColor;

}

#endif // GBUFFER


