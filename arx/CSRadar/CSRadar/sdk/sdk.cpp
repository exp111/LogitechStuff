#include "sdk.h"

ICHLClient* Client = nullptr;
IClientModeShared* ClientMode = nullptr;
IEntityList* EntityList = nullptr;
IEngineClient* EngineClient = nullptr;

player_info_t CBaseEntity::GetPlayerInfo()
{
	player_info_t info;
	EngineClient->GetPlayerInfo(GetIndex(), &info);
	return info;
}
