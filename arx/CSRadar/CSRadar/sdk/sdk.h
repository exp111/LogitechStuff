#pragma once

#include "../helpers/getvfunc.h"
#include "dt_recv2.h"
#include "CNetVars.h"
#include "vector.h"

typedef void* (__cdecl* CreateInterface_t)(const char*, int*);
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);

enum sourceLifestates
{
	LIFE_ALIVE,
	LIFE_DYING,
	LIFE_DEAD,
	LIFE_RESPAWNABLE,
	LIFE_DISCARDBODY,
};

typedef struct player_info_s
{
	__int64         unknown;            //0x0000 
	union
	{
		__int64       steamID64;          //0x0008 - SteamID64
		struct
		{
			__int32     xuid_low;
			__int32     xuid_high;
		};
	};
	char            szName[128];        //0x0010 - Player Name
	int             userId;             //0x0090 - Unique Server Identifier
	char            szSteamID[20];      //0x0094 - STEAM_X:Y:Z
	char            pad_0x00A8[0x10];   //0x00A8
	unsigned long   iSteamID;           //0x00B8 - SteamID 
	char            szFriendsName[128];
	bool            fakeplayer;
	bool            ishltv;
	unsigned int    customfiles[4];
	unsigned char   filesdownloaded;
} player_info_t;

class ClientClass
{
private:
	BYTE _chPadding[8];
public:
	char* chName;
	RecvTable* Table;
	ClientClass* pNextClass;
	int iClassID;
};

class CBaseEntity
{
public:
	ClientClass* GetClientClass()
	{
		void* pNetworkable = (void*)(this + 0x8);
		typedef ClientClass* (__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(pNetworkable, 2)(pNetworkable);
	}
	bool IsDormant()
	{
		void* pNetworkable = (void*)(this + 0x8);
		typedef bool(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(pNetworkable, 9)(pNetworkable);
	}

	//CBaseEntity
	BYTE GetLifeState()
	{
		DYNVAR_RETURN(BYTE, this, "DT_BasePlayer", "m_lifeState");
	}
	
	//CBasePlayer
	Vector GetOrigin()
	{
		DYNVAR_RETURN(Vector, this, "DT_BaseEntity", "m_vecOrigin");
	}
	int GetTeam()
	{
		DYNVAR_RETURN(int, this, "DT_BaseEntity", "m_iTeamNum");
	}

	inline bool IsAlive()
	{
		return GetLifeState() == sourceLifestates::LIFE_ALIVE;
	}
};

//Interfaces
class ICHLClient
{
public:
	ClientClass* GetAllClasses(void)
	{
		typedef ClientClass* (__thiscall* OriginalFn)(void*); //Anything inside a VTable is a __thiscall unless it completly disregards the thisptr. You can also call them as __stdcalls, but you won't have access to the __thisptr.
		return getvfunc<OriginalFn>(this, 8)(this); //Return the pointer to the head CClientClass.
	}
};

class IEntityList
{
public:
	CBaseEntity* GetClientEntity(int entNum)
	{
		typedef CBaseEntity* (__thiscall* OriginalFn)(void*, int);
		return getvfunc<OriginalFn>(this, 3)(this, entNum);
	}
	int GetHighestEntityIndex(void)
	{
		typedef int (__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(this, 6)(this);
	}
};

class IClientModeShared;

class IEngineClient
{
public:
	bool GetPlayerInfo(int ent_num, player_info_t *pinfo)
	{
		typedef bool(__thiscall* OriginalFn)(void*, int, player_info_t *);
		return getvfunc<OriginalFn>(this, 8)(this, ent_num, pinfo);
	}

	int GetLocalPlayer(void)
	{
		typedef int(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(this, 12)(this);
	}

	void GetViewAngles(Vector& va)
	{
		typedef void(__thiscall* OriginalFn)(void*, Vector& va);
		return getvfunc<OriginalFn>(this, 18)(this, va);
	}

	bool IsInGame(void)
	{
		typedef bool(__thiscall* OriginalFn)(void*);
		return getvfunc<OriginalFn>(this, 26)(this);
	}
};

extern ICHLClient* Client;
extern IEntityList* EntityList;
extern IClientModeShared* ClientMode;
extern IEngineClient* EngineClient;