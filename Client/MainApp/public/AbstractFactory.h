#pragma once
#include "GameObject.h"
#include "Button.h"

template<typename T>
class CAbstractFactory
{
public:
	CAbstractFactory() {}
	~CAbstractFactory() {}

public:
	static GameObject* Create()
	{
		GameObject* pGameObject = new T;
		pGameObject->Initialize();

		return pGameObject;
	}

	static GameObject* Create(float fX, float fY)
	{
		GameObject* pGameObject = new T;
		pGameObject->Set_Pos(fX, fY);
		pGameObject->Initialize();

		return pGameObject;
	}

	static GameObject* Create_Button(float fX, float fY, BUTTON_ID id)
	{
		GameObject* pGameObject = new T;
		dynamic_cast<Button*>(pGameObject)->SetButtonID(id);
		pGameObject->Set_Pos(fX, fY);
		pGameObject->Initialize();
		
		return pGameObject;
	}

};

