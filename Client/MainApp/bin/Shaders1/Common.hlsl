//***************************************************************************************
// Common.hlsl by Frank Luna (C) 2015 All Rights Reserved.
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

// Include structures and functions for lighting.
#include "LightingUtil.hlsl"

struct MaterialData
{
    float4 DiffuseAlbedo;
    float3 FresnelR0;
    float Roughness;
    float4x4 MatTransform;
    uint DiffuseMapIndex;
    uint NormalMapIndex;
    uint MatPad1;
    uint MatPad2;
};

struct InstanceData
{
    float4x4 WorldMat;
    uint MaterialIndex;
    uint ObjPad0;
    uint ObjPad1;
    float ObjPad2;
};

TextureCube gCubeMap : register(t0);
Texture2D gShadowMap : register(t1);

// An array of textures, which is only supported in shader model 5.1+.  Unlike Texture2DArray, the textures
// in this array can be different sizes and formats, making it more flexible than texture arrays.
Texture2D gTextureMaps[500] : register(t2);

// Put in space1, so the texture array does not overlap with these resources.  
// The texture array will occupy registers t0, t1, ..., t3 in space0. 
StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);
StructuredBuffer<InstanceData> gInstanceData : register(t1, space1);


SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
    uint gMaterialIndex;
    uint gObjPad0;
    uint gObjPad1;
    float gObjPad2;

    // Tank track bending. Filled in only for the two track meshes.
    float4 gTrackSagA;      // road wheel 1..4 vertical travel, in model units
    float4 gTrackSagB;      // road wheel 5..7, w = on/off
    float4 gTrackParam;     // x = front wheel local x, y = rear wheel local x,
                            // z = wheel axle line local z, w = top run local z

};

//---------------------------------------------------------------------------------------
// Bends the tank track so its bottom run follows the road wheels.
//
// The track mesh sits in its own space: local +x runs towards the back of the tank and
// local +z points DOWN, so the bottom run is at gTrackParam.w and the top run at .z.
// The seven road wheels sit between gTrackParam.x (front) and .y (rear); their suspension
// travel arrives in gTrackSagA/B as a model-space Y offset, so moving a vertex down means
// adding to local z. Returns the offset to add to PosL.z.
//---------------------------------------------------------------------------------------
float TrackSag(float3 posL)
{
    if (gTrackSagB.w < 0.5f)
        return 0.0f;

    float sags[7] =
    {
        gTrackSagA.x, gTrackSagA.y, gTrackSagA.z, gTrackSagA.w,
        gTrackSagB.x, gTrackSagB.y, gTrackSagB.z
    };

    float spacing = (gTrackParam.y - gTrackParam.x) / 6.0f;

    // Position along the wheel line, in wheel units (0 = front wheel, 6 = rear wheel).
    float t = (posL.x - gTrackParam.x) / spacing;

    int   i = (int) floor(clamp(t, 0.0f, 5.0f));
    float f = saturate(t - (float) i);
    float sag = lerp(sags[i], sags[i + 1], f);

    // Past the end wheels the track wraps around the sprocket and the idler, and both of
    // those are bolted to the hull - so fade the movement out over one wheel spacing.
    float endFade = saturate(t + 1.0f) * saturate(7.0f - t);

    // Everything from the wheel axle line downwards has to move with the wheel by the full
    // amount, otherwise the wheels poke out through the side of the track. Only above that
    // line does it fade out, up to the top run which rests on the return rollers and must
    // not move at all.
    float height = saturate((posL.z - gTrackParam.w) / (gTrackParam.z - gTrackParam.w));

    return -sag * endFade * height;
}

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gShadowTransform;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gAmbientLight;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
    float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
    float3 N = unitNormalW;
    float3 T = normalize(tangentW - dot(tangentW, N) * N);
    float3 B = cross(N, T);

    float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
    float3 bumpedNormalW = mul(normalT, TBN);

    return bumpedNormalW;
}

//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------

float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float) width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
    
    //// Complete projection by doing division by w.
    //shadowPosH.xyz /= shadowPosH.w;

    //float depth = shadowPosH.z;

    //// Sample only once, no filtering
    //float shadow = gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPosH.xy, depth).r;

    //return shadow;
}

