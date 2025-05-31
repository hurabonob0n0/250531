#pragma once
namespace Engine {
	template<typename T>
	unsigned long Safe_AddRef(T& Temp)
	{
		unsigned long dwRefCnt = 0;

		if (nullptr != Temp)
		{
			dwRefCnt = Temp->AddRef();
		}

		return dwRefCnt;
	}

	template<typename T>
	unsigned long Safe_Release(T& Temp)
	{
		unsigned long dwRefCnt = 0;

		if (Temp)
		{
			dwRefCnt = Temp->Release();
			if (0 == dwRefCnt)
				Temp = nullptr;
		}

		return dwRefCnt;
	}
}
