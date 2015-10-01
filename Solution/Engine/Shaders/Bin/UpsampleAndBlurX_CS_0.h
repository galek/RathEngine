#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 9.29.952.3111
//
//
//   fxc /T cs_5_0 /E UpsampleAndBlurX_CS11_0 Source/BlurX_CS.hlsl /Fh
//    Bin/UpsampleAndBlurX_CS_0.h /DKERNEL_RADIUS=0 /DHALF_RES_AO=1
//
//
// Buffer Definitions: 
//
// cbuffer GlobalConstantBuffer
// {
//
//   float2 g_FullResolution;           // Offset:    0 Size:     8
//   float2 g_InvFullResolution;        // Offset:    8 Size:     8
//   float2 g_AOResolution;             // Offset:   16 Size:     8 [unused]
//   float2 g_InvAOResolution;          // Offset:   24 Size:     8 [unused]
//   float2 g_FocalLen;                 // Offset:   32 Size:     8 [unused]
//   float2 g_InvFocalLen;              // Offset:   40 Size:     8 [unused]
//   float2 g_UVToViewA;                // Offset:   48 Size:     8 [unused]
//   float2 g_UVToViewB;                // Offset:   56 Size:     8 [unused]
//   float g_R;                         // Offset:   64 Size:     4 [unused]
//   float g_R2;                        // Offset:   68 Size:     4 [unused]
//   float g_NegInvR2;                  // Offset:   72 Size:     4 [unused]
//   float g_MaxRadiusPixels;           // Offset:   76 Size:     4 [unused]
//   float g_AngleBias;                 // Offset:   80 Size:     4 [unused]
//   float g_TanAngleBias;              // Offset:   84 Size:     4 [unused]
//   float g_PowExponent;               // Offset:   88 Size:     4 [unused]
//   float g_Strength;                  // Offset:   92 Size:     4 [unused]
//   float g_BlurDepthThreshold;        // Offset:   96 Size:     4 [unused]
//   float g_BlurFalloff;               // Offset:  100 Size:     4 [unused]
//   float g_LinA;                      // Offset:  104 Size:     4 [unused]
//   float g_LinB;                      // Offset:  108 Size:     4 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim Slot Elements
// ------------------------------ ---------- ------- ----------- ---- --------
// PointClampSampler                 sampler      NA          NA    0        1
// tAO                               texture   float          2d    0        1
// tLinDepth                         texture   float          2d    1        1
// uOutputBuffer                         UAV  float2          2d    0        1
// GlobalConstantBuffer              cbuffer      NA          NA    0        1
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue Format   Used
// -------------------- ----- ------ -------- -------- ------ ------
// no Input
//
// Output signature:
//
// Name                 Index   Mask Register SysValue Format   Used
// -------------------- ----- ------ -------- -------- ------ ------
// no Output
cs_5_0
dcl_globalFlags refactoringAllowed
dcl_constantbuffer cb0[1], immediateIndexed
dcl_sampler s0, mode_default
dcl_resource_texture2d (float,float,float,float) t0
dcl_resource_texture2d (float,float,float,float) t1
dcl_uav_typed_texture2d (float,float,float,float) u0
dcl_input vThreadGroupID.xy
dcl_input vThreadIDInGroup.x
dcl_temps 3
dcl_thread_group 320, 1, 1
utof r0.x, vThreadGroupID.x
mad r0.y, r0.x, l(320.000000), l(320.000000)
utof r0.z, vThreadIDInGroup.x
mad r1.x, r0.x, l(320.000000), r0.z
min r0.x, r0.y, cb0[0].x
lt r0.x, r1.x, r0.x
if_nz r0.x
  utof r1.y, vThreadGroupID.y
  add r0.xy, r1.xyxx, l(0.500000, 0.500000, 0.000000, 0.000000)
  mul r0.xy, r0.xyxx, cb0[0].zwzz
  sample_l_indexable(texture2d)(float,float,float,float) r0.z, r0.xyxx, t0.yzxw, s0, l(0.000000)
  sample_l_indexable(texture2d)(float,float,float,float) r2.y, r0.xyxx, t1.yxzw, s0, l(0.000000)
  ftou r1.x, r1.x
  mov r1.yzw, vThreadGroupID.yyyy
  mov r2.xzw, r0.zzzz
  store_uav_typed u0.xyzw, r1.xyzw, r2.xyzw
endif 
ret 
// Approximately 18 instruction slots used
#endif

const BYTE g_UpsampleAndBlurX_CS11_0[] =
{
     68,  88,  66,  67, 177, 169, 
     12, 151, 129,  23, 236, 225, 
     90,  76, 170, 229, 158, 136, 
    215, 226,   1,   0,   0,   0, 
     44,   9,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     28,   6,   0,   0,  44,   6, 
      0,   0,  60,   6,   0,   0, 
    144,   8,   0,   0,  82,  68, 
     69,  70, 224,   5,   0,   0, 
      1,   0,   0,   0,  32,   1, 
      0,   0,   5,   0,   0,   0, 
     60,   0,   0,   0,   0,   5, 
     83,  67,   0,   1,   0,   0, 
    175,   5,   0,   0,  82,  68, 
     49,  49,  60,   0,   0,   0, 
     24,   0,   0,   0,  32,   0, 
      0,   0,  40,   0,   0,   0, 
     36,   0,   0,   0,  12,   0, 
      0,   0,   0,   0,   0,   0, 
    220,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0, 238,   0,   0,   0, 
      2,   0,   0,   0,   5,   0, 
      0,   0,   4,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0, 242,   0, 
      0,   0,   2,   0,   0,   0, 
      5,   0,   0,   0,   4,   0, 
      0,   0, 255, 255, 255, 255, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    252,   0,   0,   0,   4,   0, 
      0,   0,   5,   0,   0,   0, 
      4,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
      1,   0,   0,   0,   5,   0, 
      0,   0,  10,   1,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  80, 111, 
    105, 110, 116,  67, 108,  97, 
    109, 112,  83,  97, 109, 112, 
    108, 101, 114,   0, 116,  65, 
     79,   0, 116,  76, 105, 110, 
     68, 101, 112, 116, 104,   0, 
    117,  79, 117, 116, 112, 117, 
    116,  66, 117, 102, 102, 101, 
    114,   0,  71, 108, 111,  98, 
     97, 108,  67, 111, 110, 115, 
    116,  97, 110, 116,  66, 117, 
    102, 102, 101, 114,   0, 171, 
     10,   1,   0,   0,  20,   0, 
      0,   0,  56,   1,   0,   0, 
    112,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     88,   4,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
      2,   0,   0,   0, 112,   4, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 148,   4, 
      0,   0,   8,   0,   0,   0, 
      8,   0,   0,   0,   2,   0, 
      0,   0, 112,   4,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 168,   4,   0,   0, 
     16,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
    112,   4,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    183,   4,   0,   0,  24,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,   0, 112,   4, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 201,   4, 
      0,   0,  32,   0,   0,   0, 
      8,   0,   0,   0,   0,   0, 
      0,   0, 112,   4,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 212,   4,   0,   0, 
     40,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
    112,   4,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    226,   4,   0,   0,  48,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,   0, 112,   4, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 238,   4, 
      0,   0,  56,   0,   0,   0, 
      8,   0,   0,   0,   0,   0, 
      0,   0, 112,   4,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 250,   4,   0,   0, 
     64,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     40,   5,   0,   0,  68,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   4,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  45,   5, 
      0,   0,  72,   0,   0,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   4,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0,  56,   5,   0,   0, 
     76,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
     74,   5,   0,   0,  80,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   4,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0,  86,   5, 
      0,   0,  84,   0,   0,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   4,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 101,   5,   0,   0, 
     88,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    115,   5,   0,   0,  92,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   4,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 126,   5, 
      0,   0,  96,   0,   0,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   4,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 147,   5,   0,   0, 
    100,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      4,   5,   0,   0,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    161,   5,   0,   0, 104,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   4,   5, 
      0,   0,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 255, 255, 255, 255, 
      0,   0,   0,   0, 168,   5, 
      0,   0, 108,   0,   0,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,   4,   5,   0,   0, 
      0,   0,   0,   0, 255, 255, 
    255, 255,   0,   0,   0,   0, 
    255, 255, 255, 255,   0,   0, 
      0,   0, 103,  95,  70, 117, 
    108, 108,  82, 101, 115, 111, 
    108, 117, 116, 105, 111, 110, 
      0, 102, 108, 111,  97, 116, 
     50,   0,   1,   0,   3,   0, 
      1,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0, 105,   4, 
      0,   0, 103,  95,  73, 110, 
    118,  70, 117, 108, 108,  82, 
    101, 115, 111, 108, 117, 116, 
    105, 111, 110,   0, 103,  95, 
     65,  79,  82, 101, 115, 111, 
    108, 117, 116, 105, 111, 110, 
      0, 103,  95,  73, 110, 118, 
     65,  79,  82, 101, 115, 111, 
    108, 117, 116, 105, 111, 110, 
      0, 103,  95,  70, 111,  99, 
     97, 108,  76, 101, 110,   0, 
    103,  95,  73, 110, 118,  70, 
    111,  99,  97, 108,  76, 101, 
    110,   0, 103,  95,  85,  86, 
     84, 111,  86, 105, 101, 119, 
     65,   0, 103,  95,  85,  86, 
     84, 111,  86, 105, 101, 119, 
     66,   0, 103,  95,  82,   0, 
    102, 108, 111,  97, 116,   0, 
      0,   0,   3,   0,   1,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0, 254,   4,   0,   0, 
    103,  95,  82,  50,   0, 103, 
     95,  78, 101, 103,  73, 110, 
    118,  82,  50,   0, 103,  95, 
     77,  97, 120,  82,  97, 100, 
    105, 117, 115,  80, 105, 120, 
    101, 108, 115,   0, 103,  95, 
     65, 110, 103, 108, 101,  66, 
    105,  97, 115,   0, 103,  95, 
     84,  97, 110,  65, 110, 103, 
    108, 101,  66, 105,  97, 115, 
      0, 103,  95,  80, 111, 119, 
     69, 120, 112, 111, 110, 101, 
    110, 116,   0, 103,  95,  83, 
    116, 114, 101, 110, 103, 116, 
    104,   0, 103,  95,  66, 108, 
    117, 114,  68, 101, 112, 116, 
    104,  84, 104, 114, 101, 115, 
    104, 111, 108, 100,   0, 103, 
     95,  66, 108, 117, 114,  70, 
     97, 108, 108, 111, 102, 102, 
      0, 103,  95,  76, 105, 110, 
     65,   0, 103,  95,  76, 105, 
    110,  66,   0,  77, 105,  99, 
    114, 111, 115, 111, 102, 116, 
     32,  40,  82,  41,  32,  72, 
     76,  83,  76,  32,  83, 104, 
     97, 100, 101, 114,  32,  67, 
    111, 109, 112, 105, 108, 101, 
    114,  32,  57,  46,  50,  57, 
     46,  57,  53,  50,  46,  51, 
     49,  49,  49,   0,  73,  83, 
     71,  78,   8,   0,   0,   0, 
      0,   0,   0,   0,   8,   0, 
      0,   0,  79,  83,  71,  78, 
      8,   0,   0,   0,   0,   0, 
      0,   0,   8,   0,   0,   0, 
     83,  72,  69,  88,  76,   2, 
      0,   0,  80,   0,   5,   0, 
    147,   0,   0,   0, 106,   8, 
      0,   1,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
     90,   0,   0,   3,   0,  96, 
     16,   0,   0,   0,   0,   0, 
     88,  24,   0,   4,   0, 112, 
     16,   0,   0,   0,   0,   0, 
     85,  85,   0,   0,  88,  24, 
      0,   4,   0, 112,  16,   0, 
      1,   0,   0,   0,  85,  85, 
      0,   0, 156,  24,   0,   4, 
      0, 224,  17,   0,   0,   0, 
      0,   0,  85,  85,   0,   0, 
     95,   0,   0,   2,  50,  16, 
      2,   0,  95,   0,   0,   2, 
     18,  32,   2,   0, 104,   0, 
      0,   2,   3,   0,   0,   0, 
    155,   0,   0,   4,  64,   1, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,  86,   0, 
      0,   4,  18,   0,  16,   0, 
      0,   0,   0,   0,  10,  16, 
      2,   0,  50,   0,   0,   9, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 160,  67, 
      1,  64,   0,   0,   0,   0, 
    160,  67,  86,   0,   0,   4, 
     66,   0,  16,   0,   0,   0, 
      0,   0,  10,  32,   2,   0, 
     50,   0,   0,   9,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   0,   0, 
      0,   0,   1,  64,   0,   0, 
      0,   0, 160,  67,  42,   0, 
     16,   0,   0,   0,   0,   0, 
     51,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     26,   0,  16,   0,   0,   0, 
      0,   0,  10, 128,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  49,   0,   0,   7, 
     18,   0,  16,   0,   0,   0, 
      0,   0,  10,   0,  16,   0, 
      1,   0,   0,   0,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     31,   0,   4,   3,  10,   0, 
     16,   0,   0,   0,   0,   0, 
     86,   0,   0,   4,  34,   0, 
     16,   0,   1,   0,   0,   0, 
     26,  16,   2,   0,   0,   0, 
      0,  10,  50,   0,  16,   0, 
      0,   0,   0,   0,  70,   0, 
     16,   0,   1,   0,   0,   0, 
      2,  64,   0,   0,   0,   0, 
      0,  63,   0,   0,   0,  63, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  56,   0,   0,   8, 
     50,   0,  16,   0,   0,   0, 
      0,   0,  70,   0,  16,   0, 
      0,   0,   0,   0, 230, 138, 
     32,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,  72,   0, 
      0, 141, 194,   0,   0, 128, 
     67,  85,  21,   0,  66,   0, 
     16,   0,   0,   0,   0,   0, 
     70,   0,  16,   0,   0,   0, 
      0,   0, 150, 124,  16,   0, 
      0,   0,   0,   0,   0,  96, 
     16,   0,   0,   0,   0,   0, 
      1,  64,   0,   0,   0,   0, 
      0,   0,  72,   0,   0, 141, 
    194,   0,   0, 128,  67,  85, 
     21,   0,  34,   0,  16,   0, 
      2,   0,   0,   0,  70,   0, 
     16,   0,   0,   0,   0,   0, 
     22, 126,  16,   0,   1,   0, 
      0,   0,   0,  96,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0,   0,   0, 
     28,   0,   0,   5,  18,   0, 
     16,   0,   1,   0,   0,   0, 
     10,   0,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   4, 
    226,   0,  16,   0,   1,   0, 
      0,   0,  86,  21,   2,   0, 
     54,   0,   0,   5, 210,   0, 
     16,   0,   2,   0,   0,   0, 
    166,  10,  16,   0,   0,   0, 
      0,   0, 164,   0,   0,   7, 
    242, 224,  17,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      1,   0,   0,   0,  70,  14, 
     16,   0,   2,   0,   0,   0, 
     21,   0,   0,   1,  62,   0, 
      0,   1,  83,  84,  65,  84, 
    148,   0,   0,   0,  18,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   1,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   5,   0,   0,   0, 
      0,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0
};
