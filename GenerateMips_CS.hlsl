#define BLOCK_SIZE 8
#define WIDTH_HEIGHT_EVEN 0 //Both the width and the height of the texture are even
#define WIDTH_ODD_HEIGHT_EVEN 1 //Texture width is odd and height is even
#define WIDTH_EVEN_HEIGHT_ODD 2 //Texture width is even, height is odd
#define WIDTH_HEIGHT_ODD 3 //Both the width and height of the texture are odd

struct ComputeShaderInput
{
    uint3 GroupID           : SV_GroupID; //3D index of the thread group in the dispatch
    uint3 GroupThreadID     : SV_GroupThreadID; //3D index of local thread ID in a thread group
    uint3 DispatchThreadID  : SV_DispatchThreadID; //3D index of global thread ID in the dispatch
    uint  GroupIndex        : SV_GroupIndex; //The flattened local index of the thread within a thread group
};

cbuffer GenerateMipsCB : register(b0)
{
    uint SrcMipLevel; //Texture lvl of source mip
    uint NumMipLevels; //Number of OutMips to write: [1-4]
    uint SrcDimension; //Width and height of the source texture are even or odd
    bool IsSRGB; //Must apply the gamma correction to sRGB textures
    float2 TexelSize; //1.0 / OutMip1.Dimensions
}

//Source mip map
Texture2D<float4> SrcMip : register(t0);

//Write up to 4 mip map levels
RWTexture2D<float4> OutMip1 : register(u0);
RWTexture2D<float4> OutMip2 : register(u1);
RWTexture2D<float4> OutMip3 : register(u2);
RWTexture2D<float4> OutMip4 : register(u3);

//Linear clamp sampler
SamplerState LinearClampSampler : register(s0);

#define GenerateMips_RootSignature \
"RootFlags(0), " \
"RootConstants(b0, num32BitConstants = 6), " \
"DescriptorTable(SRV(t0, numDescriptors = 1) ), " \
"DescriptorTable(UAV(u0, numDescriptors = 4) ), " \
"StaticSampler(s0, " \
"addressU = TEXTURE_ADDRESS_CLAMP," \
"addressV = TEXTURE_ADDRESS_CLAMP," \
"addressW = TEXTURE_ADDRESS_CLAMP," \
"filter = FILTER_MIN_MAG_MIP_LINEAR)"

//The reason for separating channels is to reduce bank conflicts in the local data memory controller
//A large stride will cause more threads to collide on the same memory bank
groupshared float gs_R[64];
groupshared float gs_G[64];
groupshared float gs_B[64];
groupshared float gs_A[64];

void StoreColor(uint Index, float4 Color)
{
    gs_R[Index] = Color.r;
    gs_G[Index] = Color.g;
    gs_B[Index] = Color.b;
    gs_A[Index] = Color.a;
}

float4 LoadColor(uint Index)
{
    return float4(gs_R[Index], gs_G[Index], gs_B[Index], gs_A[Index]);
}

float3 ConvertToLinear(float3 x)
{
    return x < 0.04045f ? x / 12.92 : pow((x + 0.055) / 1.055, 2.4);
}

float3 ConvertToSRGB(float3 x)
{
    return x < 0.0031308 ? 12.92 * x : 1.055 * pow(abs(x), 1.0 / 2.4) - 0.055;
}

//Convert linear color to sRGB before storing if the original source is an sRGB texture
float4 PackColor(float4 x)
{
    if(IsSRGB)
    {
        return float4(ConvertToSRGB(x.rgb), x.a);
    }
    else
    {
        return x;
    }
}

[RootSignature(GenerateMips_RootSignature)]
[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]

void main( ComputeShaderInput IN )
{
    //Stores the color value for the mip level currently being generated
    float4 Src1 = (float4) 0;
    
    //One bilinear sample is insufficient when scaling down by more than 2x
    //You'll slightly undersample in the case where the source dimension is odd.
    //Ideally only use power-of-two sized textures. Handling undersampling is slow and complicated
    
    //Determine the path to use based on the dimension of the source texture
    //0b00(0): Both width and height are even
    //0b01(1): width is odd, height is even
    //0b10(2): width is even, height is odd
    //0b11(3): Both width and height are odd
    switch (SrcDimension)
    {
        case WIDTH_HEIGHT_EVEN:
        {
                float2 UV = TexelSize * (IN.DispatchThreadID.xy + 0.5);
            
                Src1 = SrcMip.SampleLevel(LinearClampSampler, UV, SrcMipLevel);
        }
        break;
        case WIDTH_ODD_HEIGHT_EVEN:
        {
            // > 2:1 in X dimension, use 2 bilinear samples to gurantee we don't undersample when downsizing by more than 2x horizontally
            float2 UV1 = TexelSize * (IN.DispatchThreadID.xy + float2(0.25, 0.5));
            float2 Off = TexelSize * float2(0.5, 0.0);
            
                Src1 = 0.5 * (SrcMip.SampleLevel(LinearClampSampler, UV1, SrcMipLevel) + SrcMip.SampleLevel(LinearClampSampler, UV1 + Off, SrcMipLevel));
        }
        break;
        case WIDTH_EVEN_HEIGHT_ODD:
        {
            // > 2:1 in Y dimension, use 2 bilinear samples to gurantee we don't undersample when downsizing by more than 2x vertically
                float2 UV1 = TexelSize * (IN.DispatchThreadID.xy + float2(0.5, 0.25));
                float2 Off = TexelSize * float2(0.0, 0.5);
            
                Src1 = 0.5 * (SrcMip.SampleLevel(LinearClampSampler, UV1, SrcMipLevel) + SrcMip.SampleLevel(LinearClampSampler, UV1 + Off, SrcMipLevel));
            }
            break;
        case WIDTH_HEIGHT_ODD:
        {
            // > 2:1 in both dimensions
            //Use 4 bilinear samples to gurantee we don't undersample when downsizing by more than 2x in both directions
                float2 UV1 = TexelSize * (IN.DispatchThreadID.xy + float2(0.25, 0.25));
                float2 Off = TexelSize * 0.5;
            
                Src1 = SrcMip.SampleLevel(LinearClampSampler, UV1, SrcMipLevel);
                Src1 += SrcMip.SampleLevel(LinearClampSampler, UV1 + float2(Off.x, 0.0), SrcMipLevel);
                Src1 += SrcMip.SampleLevel(LinearClampSampler, UV1 + float2(0.0, Off.y), SrcMipLevel);
                Src1 += SrcMip.SampleLevel(LinearClampSampler, UV1 + float2(Off.x, Off.y), SrcMipLevel);
                Src1 *= 0.25;
            }
            break;
    }
    
    OutMip1[IN.DispatchThreadID.xy] = PackColor(Src1);
    
    //A scalar(constant) branch can exil all threads coherently
    if(NumMipLevels == 1)
        return;
    
    //Without lane swizzle operations, the only way to share data with other threads is through LDS
    StoreColor(IN.GroupIndex, Src1);
    
    //This gurantees all LDS writes are complete and that all threads have executed all instructions so far (and therefore have issued their LDS write instructions)
    GroupMemoryBarrierWithGroupSync();
    
    //With low three bits for X and high three bits for Y, this bit mask (binary: 001001) checks that X and Y are even
    if((IN.GroupIndex & 0x9) == 0)
    {
        float4 Src2 = LoadColor(IN.GroupIndex + 0x01);
        float4 Src3 = LoadColor(IN.GroupIndex + 0x08);
        float4 Src4 = LoadColor(IN.GroupIndex + 0x09);
        Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);
        
        OutMip2[IN.DispatchThreadID.xy / 2] = PackColor(Src1);
        StoreColor(IN.GroupIndex, Src1);
    }
    if (NumMipLevels == 2)
        return;
    
    GroupMemoryBarrierWithGroupSync();
    
    //This bit mask (binary: 011011) checks that X and Y are multiples of four
    if((IN.GroupIndex & 0x1B) == 0)
    {
        float4 Src2 = LoadColor(IN.GroupIndex + 0x02);
        float4 Src3 = LoadColor(IN.GroupIndex + 0x10);
        float4 Src4 = LoadColor(IN.GroupIndex + 0x12);
        Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);
        
        OutMip3[IN.DispatchThreadID.xy / 4] = PackColor(Src1);
        StoreColor(IN.GroupIndex, Src1);
    }
    if(NumMipLevels == 3)
        return;
    
    GroupMemoryBarrierWithGroupSync();

    //This bit mask would be 111111 (X & Y multiples of 8), but only one thread fits that criteria
    if (IN.GroupIndex == 0)
    {
        float4 Src2 = LoadColor(IN.GroupIndex + 0x04);
        float4 Src3 = LoadColor(IN.GroupIndex + 0x20);
        float4 Src4 = LoadColor(IN.GroupIndex + 0x24);
        Src1 = 0.25 * (Src1 + Src2 + Src3 + Src4);
        
        OutMip4[IN.DispatchThreadID.xy / 8] = PackColor(Src1);
    }
}