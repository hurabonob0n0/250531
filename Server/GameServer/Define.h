#pragma once


#define PURE	= 0

/*----------------
	For Object
----------------*/

enum ObjectID {

	OBJ_TANK, OBJ_END

};

struct Position {

	float PosX;
	float PosY;
	float PosZ;

};

template<typename T>
void Safe_Delete(T& Temp)
{
	if (Temp)
	{
		delete Temp;
		Temp = nullptr;
	}
}



/*------------------
	For Protocol
------------------*/


