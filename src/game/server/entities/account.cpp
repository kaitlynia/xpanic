/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#include <string.h>
#include <fstream>
#include <engine/config.h>
#include <engine/shared/config.h>
#include "account.h"
#include <engine/server/crypt.h>


CAccount::CAccount(CPlayer *pPlayer, CGameContext *pGameServer)
{
	m_pPlayer = pPlayer;
	m_pGameServer = pGameServer;
}

void CAccount::Login(char *Username, char *Password)
{
	char aBuf[125];
	if (m_pPlayer->m_AccData.m_UserID)
		return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Already logged in");

	char aHash[64]; //Result
	mem_zero(aHash, sizeof(aHash));
	Crypt(Password, (const unsigned char*) "d9", 1, 14, aHash);

	GameServer()->Login(Username, aHash, m_pPlayer->GetCID());
}

void CAccount::Register(char *Username, char *Password)
{
	char aBuf[125];
	if(m_pPlayer->m_AccData.m_UserID)
		return GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Already logged in");

	GameServer()->Register(Username, Password, m_pPlayer->GetCID());
}

bool CAccount::Exists(const char *Username)
{
}

bool CAccount::Apply()
{
	return GameServer()->Apply(m_pPlayer->m_AccData.m_Username, m_pPlayer->m_AccData.m_Password, m_pPlayer->m_AccData.m_UserID, m_pPlayer->m_AccData.m_PlayerState, m_pPlayer->m_AccData.m_Level, m_pPlayer->m_AccData.m_Exp, m_pPlayer->m_AccData.m_Money, m_pPlayer->m_AccData.m_Dmg, m_pPlayer->m_AccData.m_Health, m_pPlayer->m_AccData.m_Ammoregen, m_pPlayer->m_AccData.m_Handle, m_pPlayer->m_AccData.m_Ammo, m_pPlayer->m_AccData.m_TurretMoney, m_pPlayer->m_AccData.m_TurretLevel, m_pPlayer->m_AccData.m_TurretExp, m_pPlayer->m_AccData.m_TurretDmg, m_pPlayer->m_AccData.m_TurretSpeed, m_pPlayer->m_AccData.m_TurretSpeed, m_pPlayer->m_AccData.m_TurretShotgun, m_pPlayer->m_AccData.m_TurretRange, m_pPlayer->m_AccData.m_Freeze, m_pPlayer->m_AccData.m_Winner, m_pPlayer->m_AccData.m_Luser);
}

void CAccount::Reset()
{
	m_pPlayer->m_AccData.m_UserID = 0;
	str_copy(m_pPlayer->m_AccData.m_Username, "", 32);
	str_copy(m_pPlayer->m_AccData.m_Password, "", 32);
	
	m_pPlayer->m_AccData.m_Exp = m_pPlayer->m_AccData.m_Level = m_pPlayer->m_AccData.m_Money = 0;
	m_pPlayer->m_AccData.m_Dmg = m_pPlayer->m_AccData.m_Health = m_pPlayer->m_AccData.m_Ammoregen = m_pPlayer->m_AccData.m_Handle = m_pPlayer->m_AccData.m_Ammo = 0;
	m_pPlayer->m_AccData.m_PlayerState = 0;
	m_pPlayer->m_AccData.m_TurretMoney = m_pPlayer->m_AccData.m_TurretLevel = m_pPlayer->m_AccData.m_TurretExp = m_pPlayer->m_AccData.m_TurretDmg = m_pPlayer->m_AccData.m_TurretSpeed = 0;
	m_pPlayer->m_AccData.m_TurretAmmo = m_pPlayer->m_AccData.m_TurretShotgun = m_pPlayer->m_AccData.m_TurretRange = m_pPlayer->m_AccData.m_Freeze = 0;
}

void CAccount::NewPassword(char *NewPassword)
{
	if(str_length(NewPassword) > 12 || str_length(NewPassword) < 4)
	{
		char aBuf[48];
		str_format(aBuf, sizeof(aBuf), "Password too %s!", str_length(NewPassword)<4?"long":"short");
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		return;
    }
	
	char Filter[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.-_";
	char *p = strpbrk(NewPassword, Filter);
	if(!p)
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Don't use invalid chars for password!");
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), "A - Z, a - z, 0 - 9, . - _");
		return;
	}
	
	str_copy(m_pPlayer->m_AccData.m_Password, NewPassword, 32);
	Apply();
	
	dbg_msg("account", "Password changed - ('%s')", m_pPlayer->m_AccData.m_Username);
	GameServer()->SendChatTarget(m_pPlayer->GetCID(), "Password successfully changed!");
}


int CAccount::NextID()
{
}
