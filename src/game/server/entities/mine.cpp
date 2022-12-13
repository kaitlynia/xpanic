#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>

#include "mine.h"

CMine::CMine(CGameWorld *pGameWorld, vec2 Pos, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_PROJECTILE)
{
	m_Pos = Pos;
	m_Owner = Owner;

	GameWorld()->InsertEntity(this);
}

bool CMine::HitCharacter()
{
	bool Result = false;
	CCharacter *apCloseCCharacters[MAX_CLIENTS];
	if (const int Num = GameServer()->m_World.FindEntities(m_Pos, 8.0f, (CEntity**)apCloseCCharacters, MAX_CLIENTS, CGameWorld::ENTTYPE_CHARACTER))
	{
		for (int i = 0; i < Num; i++)
		{
			if (apCloseCCharacters[i]->GetPlayer()->GetTeam() == TEAM_RED)
				continue;

			const float Len = distance(apCloseCCharacters[i]->m_Pos, m_Pos);
			if (Len < apCloseCCharacters[i]->m_ProximityRadius + 2.0f)
			{
				Result = true;
				apCloseCCharacters[i]->m_BurnTick = Server()->TickSpeed() * 2;
			}
		}
	}
	return Result;
}

void CMine::Tick()
{	
	if (!GameServer()->GetPlayerChar(m_Owner) || GameServer()->m_apPlayers[m_Owner]->GetTeam() != TEAM_RED || HitCharacter())
	{
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_EXPLODE);
		GameWorld()->DestroyEntity(this);
	}
}

void CMine::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Projectile *pObj = static_cast<CNetObj_Projectile *>(Server()->SnapNewItem(NETOBJTYPE_PROJECTILE, m_ID, sizeof(CNetObj_Projectile)));
	if(!pObj)
		return;

	pObj->m_X = (int)m_Pos.x;
	pObj->m_Y = (int)m_Pos.y;
	pObj->m_VelX = 1;
	pObj->m_VelY = 1;
	pObj->m_Type = WEAPON_RIFLE;
}
