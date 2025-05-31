#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace Engine 
{
	typedef char						_char;
	typedef wchar_t						_tchar;

	typedef unsigned char				_ubyte;
	typedef signed char					_byte;

	typedef unsigned short				_ushort;
	typedef signed short				_short;

	typedef unsigned int				_uint;
	typedef signed int					_int;

	typedef unsigned long				_ulong;
	typedef signed long					_long;

	typedef float						_float;
	typedef double						_double;

	typedef bool						_bool;

	typedef XMFLOAT2					_float2;
	typedef XMFLOAT3					_float3;
	typedef XMFLOAT4					_float4;
	typedef XMVECTOR					_vector;
	typedef FXMVECTOR					_fvector;
	typedef GXMVECTOR					_gvector;

	typedef XMFLOAT4X4					_float4x4;
	typedef XMMATRIX					_matrix;
	typedef FXMMATRIX					_fmatrix;

}