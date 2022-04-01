#pragma once


namespace hyd
{
    
enum VertexFormat : uint8
{
    VF_Unknown,

    VF_P3F_C4B_T2F, // simple
    VF_P3F_C4B_T2F_W4B_I4S,  // Skinned weights/indices stream.
    VF_P2F_C4B_T2F_F4B, // UI
    
    VF_Max,
};

} // namespace hyd
