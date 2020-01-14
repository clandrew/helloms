// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <initguid.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <initguid.h>
#include <dxgidebug.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <exception>
#include <string>

#include <wrl/client.h>
#include <comdef.h>

#include "Common/d3dx12.h"

// Helper define, to be expected out of d3dx12.h at some point.
typedef CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT< D3D12_SHADER_BYTECODE, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>  CD3DX12_PIPELINE_STATE_STREAM_MS;

#endif //PCH_H
