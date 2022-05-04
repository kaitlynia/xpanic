#include <stdio.h>
#include <string.h>

#include <engine/shared/config.h>
#include <engine/server.h>
#include <game/version.h>
#include "cmds.h"
#include "account.h"
#include "hearth.h"

#include <teeothers/components/localization.h>
CCmd::CCmd(CPlayer *pPlayer, CGameContext *pGameServer)
{
	m_pPlayer = pPlayer;
	m_pGameServer = pGameServer;
}

void CCmd::ChatCmd(CNetMsg_Cl_Say *Msg)
{
	if(!strncmp(Msg->m_pMessage, "/login", 6))
	{
		LastChat();		
		if(GameServer()->m_World.m_Paused) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please wait for the round to end!"));
		
		char Username[256], Password[256];
		if(sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use: /login <username> <password>"));
		
		if(str_length(Username) > 15 || str_length(Username) < 4 || str_length(Password) > 15 || str_length(Password) < 4) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Username / Password must contain 4-15 characters"));
		
		m_pPlayer->m_pAccount->Login(Username, Password, m_pPlayer->GetCID());
		return;
	}
	
	else if(!strncmp(Msg->m_pMessage, "/register", 9))
	{
		LastChat();
		if(GameServer()->m_World.m_Paused)  return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please wait for the round to end!"));
		
		char Username[256], Password[256];
		if(sscanf(Msg->m_pMessage, "/register %s %s", Username, Password) != 2) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use /register <username> <password>'"));
		
		if(str_length(Username) > 15 || str_length(Username) < 4 || str_length(Password) > 15 || str_length(Password) < 4)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Username and Password must contain 4-15 characters"));
		else if (!strcmp(Username, Password))
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Username and Password must be different"));

		m_pPlayer->m_pAccount->Register(Username, Password, m_pPlayer->GetCID());
		return;
	}
	
	else if (!strncmp(Msg->m_pMessage, "/sd", 3) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int size = 0;
		if ((sscanf(Msg->m_pMessage, "/sd %d", &size)) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use /sd <idsound>"));

		if (m_pPlayer->GetTeam() == TEAM_SPECTATORS || !GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
			return;

		int soundid = clamp(size, 0, 40);
		GameServer()->CreateSound(GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_Pos, soundid);
		return;
	}

	else if(!strcmp(Msg->m_pMessage, "/logout"))
	{
		LastChat();
		if(!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You aren't logged in!"));
		if(GameServer()->m_World.m_Paused) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please wait for the round to end!"));
		if (GameServer()->m_pController->NumZombs() == 1 && m_pPlayer->GetTeam() == TEAM_RED) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You can't log out as the last zombie!"));
		if (GameServer()->m_pController->NumPlayers() < 3 && GameServer()->m_pController->m_Warmup)	return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Wait for the round to start!"));

		m_pPlayer->m_pAccount->Apply(), m_pPlayer->m_pAccount->Reset();

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Logged out."));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use /login <username> <password> to login again"));

		if(GameServer()->GetPlayerChar(m_pPlayer->GetCID()) && GameServer()->GetPlayerChar(m_pPlayer->GetCID())->IsAlive())
			GameServer()->GetPlayerChar(m_pPlayer->GetCID())->Die(m_pPlayer->GetCID(), WEAPON_GAME);

		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/upgr", 5))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Use /account for more information"));

		char supgr[256], andg[64];
		if (sscanf(Msg->m_pMessage, "/upgr %s", supgr) != 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use /upgr <dmg>, <hp>, <handle>, <ammoregen>, <ammo>, <stats> to upgrade stats"));
			return;
		}
		if (!strcmp(supgr, "handle"))
		{
			if (m_pPlayer->m_AccData.m_Handle >= 300)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximum handle level already reached (300)."));
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient money (1 needed)."));

			m_pPlayer->m_AccData.m_Money--, m_pPlayer->m_AccData.m_Handle++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("New handle: {int:handle}, Your money: {int:money}"), "handle", &m_pPlayer->m_AccData.m_Handle, "money", &m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "dmg"))
		{
			if (m_pPlayer->m_AccData.m_Dmg >= 21)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximum damage level already reached (21)."));
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient money (1 needed)."));

			m_pPlayer->m_AccData.m_Money--, m_pPlayer->m_AccData.m_Dmg++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("New damage: {int:damage}, Your money: {int:Money}"), "damage", &m_pPlayer->m_AccData.m_Dmg, "Money", &m_pPlayer->m_AccData.m_Money);m_pPlayer->m_pAccount->Apply();
			
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "hp"))
		{
			if (m_pPlayer->m_AccData.m_Health >= 100)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximum HP level already reached (100)."));
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient money (1 needed)."));

			m_pPlayer->m_AccData.m_Money--, m_pPlayer->m_AccData.m_Health++;
			int MaxHP = 1000 + m_pPlayer->m_AccData.m_Health * 10 + m_pPlayer->m_AccData.m_Level * 20;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("New HP: {int:HP}, Maximum HP: {int:MaximumHP}, Your money: {int:Money}"), 
			"HP", &m_pPlayer->m_AccData.m_Health, 
			"MaximumHP", &MaxHP, 
			"Money", &m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "ammoregen"))
		{
			if (m_pPlayer->m_AccData.m_Ammoregen >= 60)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximum ammo regen level already reached (60)."));
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient money (1 needed)."));

			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("New ammo regen: {int:Ammoregen}, Your money: {int:Money}"), "Ammoregen", &m_pPlayer->m_AccData.m_Ammoregen, "Money", &m_pPlayer->m_AccData.m_Money);m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "ammo"))
		{
			if (m_pPlayer->m_AccData.m_Ammo >= 20)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximum ammo level already reached (20)."));
			if (m_pPlayer->m_AccData.m_Money < 10)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient money (10 needed)."));

			m_pPlayer->m_AccData.m_Money -= 10, m_pPlayer->m_AccData.m_Ammo++;
			int Ammo = GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_mAmmo + m_pPlayer->m_AccData.m_Ammo;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("New ammo: {int:Ammo}, Your money: {int:Money}"), "Ammo", &Ammo, "Money", &m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "stats"))
		{
			char aBuf2[32];
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ ! Upgr stats details ! ~~~~~~~~"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Damage: {int:dmg}"), "dmg", &m_pPlayer->m_AccData.m_Dmg);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("HP: {int:hp}"), "hp", &m_pPlayer->m_AccData.m_Health);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Handle: {int:handle}"), "handle", &m_pPlayer->m_AccData.m_Handle);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Ammo regen: {int:ammoregen}"), "ammoregen", &m_pPlayer->m_AccData.m_Ammoregen);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Ammo: {int:ammo}"), "ammo", &m_pPlayer->m_AccData.m_Ammo);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ ! Upgr stats details ! ~~~~~~~~"));
			return;
		}
		else return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("This type doesn't exist."));
	}
	else if(!strncmp(Msg->m_pMessage, "/turret", 7))
	{
		LastChat();
		if(!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Use /account for more information!"));

		char supgr[256], andg[64];
		if (sscanf(Msg->m_pMessage, "/turret %s", supgr) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use /turret info"));

		if(!strcmp(supgr, "info"))
		{		
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret info - show information"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret help - show help"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret stats - view stats upgrade turrets"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret speed - upgrade turret speed (1 turret money)"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret dmg - upgrade turret damage (1 turret money)"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret ammo - upgrade turret ammo (1 turret money)"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret ammoregen - upgrade turret ammo (1 turret money)"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret range - upgrade turret range (1 turret money)"));
			return;
		}			
		else if(!strcmp(supgr, "help"))
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Turret could be placed via Ghost Emoticon"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Turrets can have 3 different weapons"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Gun Turret: Shoots once per sec at enemy"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Shotgun Turret: Shoots once per 3 sec at enemy"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Laser Turret: places a laser each 40 sec if the zombie reach it's line"));
			return;
		}
		else if(!strcmp(supgr, "dmg"))
		{
			if(m_pPlayer->m_AccData.m_TurretDmg > 100)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximal turret damage level!"));		
			if(m_pPlayer->m_AccData.m_TurretMoney <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You have not turret money counts!"));
			
			m_pPlayer->m_AccData.m_TurretMoney--, m_pPlayer->m_AccData.m_TurretDmg++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), ("Your turret's damage is upgraded, now it is: {int:TurretDmg}"), "TurretDmg", &m_pPlayer->m_AccData.m_TurretDmg);
			m_pPlayer->m_pAccount->Apply();
			return;
		}			
		else if(!strcmp(supgr, "speed"))
		{
			if(m_pPlayer->m_AccData.m_TurretSpeed >= 500)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximal turret speed level!"));		
			if(m_pPlayer->m_AccData.m_TurretMoney <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You have not turret money counts!"));
			
			m_pPlayer->m_AccData.m_TurretMoney--, m_pPlayer->m_AccData.m_TurretSpeed++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Your turret's speed is upgraded, now it is: {int:TurretSpeed}"), "TurretSpeed", &m_pPlayer->m_AccData.m_TurretSpeed);
			m_pPlayer->m_pAccount->Apply();
			return;
		}	
		else if (!strcmp(supgr, "ammo"))
		{
			if (m_pPlayer->m_AccData.m_TurretAmmo >= 100)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximal turret ammo level!"));
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You have not turret money counts!"));

			m_pPlayer->m_AccData.m_TurretMoney--, m_pPlayer->m_AccData.m_TurretAmmo++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Your turret's gun is upgraded, now it is: {int:TurretAmmo}"), "TurretAmmo", m_pPlayer->m_AccData.m_TurretAmmo);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "ammoregen"))
		{
			if (m_pPlayer->m_AccData.m_TurretShotgun >= 75)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximal turret ammoregen level!"));
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You have not turret money counts!"));

			m_pPlayer->m_AccData.m_TurretMoney--, m_pPlayer->m_AccData.m_TurretShotgun++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Your turret's ammoregen is upgraded, now it is: {int:TurretShotgun}"), "TurretShotgun", &m_pPlayer->m_AccData.m_TurretShotgun);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "range"))
		{
			if (m_pPlayer->m_AccData.m_TurretRange >= 200)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Maximal turret range level!"));
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
				return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You have not turret money counts!"));

			m_pPlayer->m_AccData.m_TurretMoney--, m_pPlayer->m_AccData.m_TurretRange++;
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Your turret's range is upgraded, now it is: {int:TurretRange}"), "TurretRange", &m_pPlayer->m_AccData.m_TurretRange);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "stats"))
		{
			char aBuf[64];
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("---- Turret ----"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Lvl: {int:Lvl}"), "Lvl", m_pPlayer->m_AccData.m_TurretLevel);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Exp: {int:Exp}"), "Exp", m_pPlayer->m_AccData.m_TurretExp);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Money: {int:Money}"), "Money", m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Speed: {int:Speed}"), "Speed", m_pPlayer->m_AccData.m_TurretSpeed);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Damage: {int:Damage}"), "Damage", m_pPlayer->m_AccData.m_TurretDmg);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Ammo: {int:Ammo}"), "Ammo", m_pPlayer->m_AccData.m_TurretAmmo);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("AmmoRegen: {int:AmmoRegen}"), "AmmoRegen", m_pPlayer->m_AccData.m_TurretShotgun);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Range: {int:Range}"), "Range", m_pPlayer->m_AccData.m_TurretRange);
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("---- Turret ----"));
			return;
		}
		else
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("No such turret upgrade: '{str:wtf}'!"), "wtf", supgr);
			return;
		}
	}
	else if (!str_comp_nocase(Msg->m_pMessage, "/me") || !str_comp_nocase(Msg->m_pMessage, "/status") || !str_comp_nocase(Msg->m_pMessage, "/stats"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Type '/account' for more information!"));

		char aBuf[64];
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ ! Your account details ! ~~~~~~~~"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Login: {str:Username}"), "Username", m_pPlayer->m_AccData.m_Username);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Password: {str:Password}"), "Password", m_pPlayer->m_AccData.m_Password);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Mailbox: <no mailbox> (no active)"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Level: {int:Level}"), "Level", &m_pPlayer->m_AccData.m_Level);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Exp: {int:Exp}"), "Exp", &m_pPlayer->m_AccData.m_Exp);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Money: {int:Money}"), "Money", &m_pPlayer->m_AccData.m_Money);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Freeze: {int:Freeze}"), "Freeze", &m_pPlayer->m_AccData.m_Freeze);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You can change your password with the command:"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/password <new password>"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ ! Your account details ! ~~~~~~~~"));
		return;
	}
	else if(!strncmp(Msg->m_pMessage, "/idlist", 7))
	{
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("--- List IDs ---"));
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if(GameServer()->m_apPlayers[i])
			{
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "%s ID:%i", GameServer()->Server()->ClientName(i), i);
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
			}	
		}
		return;
	}
	else if(!strncmp(Msg->m_pMessage, "/password", 9))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Type '/account' for more information!"));

		char NewPassword[256];
		if(sscanf(Msg->m_pMessage, "/password %s", NewPassword) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use \"/password <new password>\""));

		m_pPlayer->m_pAccount->NewPassword(NewPassword);
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/vip"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("--- ViP ---"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("ViP: Get 3x Exp & 2x Money & 5 For killing spree"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Invis: +1 secounds, InvisCD: 20 secounds"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/prefix = +5 ammo + Special Dmg & 5 For killing spree"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("1 month VIP = 0euros || 3 months VIP = 0euros || LifeTime VIP = 0euros"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Payments: Name, name"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Skype: nope, Name: nope"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/supervip"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("--- SuperViP ---"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("SuperVIP: Get 5x Exp & 3x Money & 5 Score4Life & 3 For killing spree"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Invis: +3 secounds, InvisCD: 10 secounds"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/superprefix = Special Dmg + 15 Ammo, Access to commands:"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/resetupgr - Reset your upgr & /supervipmsg - Special messge"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("1 month SuperVIP = 0euros || 3 months UltimateVIP = 0euros || LifeTime UltimateVIP = 0euros"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Payments: Name, name"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Skype: nope, Name: nope"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/ultimatevip"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("--- UltimateViP ---"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("UltimateVIP: Get 7x Exp & 5x Money & 5 Score4Life & 2 For killing spree"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Invis: +5 secounds, InvisCD: 5 secounds"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/resetupgr - Reset your upgr & /ultimatemsg - Special message"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("1 month UltimateVIP = 0euros || 3 months UltimateVIP = 0euros || LifeTime UltimateVIP = 0euros"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Payments: Name, name"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Skype: nope, Name: nope"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/info"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ Welcome to info! ~~~~~~~~"));
		char aBuf[128];
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Owner: {str:Owner} / Skype: {str:Skype}"), "Owner", g_Config.m_SvOwnerName, "Skype", g_Config.m_SvOwnerSkype);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Panic mod by Kurosio, helper: {str:name}"), "name", g_Config.m_SvHelperName);
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ Welcome to info! ~~~~~~~~"));
		return;
	}
	else if(!strcmp(Msg->m_pMessage, "/help"))
    {
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ Welcome to help! ~~~~~~~~"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/account - information & help account system"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/news - for see what news in server"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/rules - for read the rules Panic"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/pm - send personal message to player"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/cmdlist - commands server"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/vip - get info vip status"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/supervip - get info supervip status"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/ultimatevip - get info ultimatevip status"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/turret info - info about turrets"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/levels - info about level"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/shop - shop score tees"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ Welcome to help! ~~~~~~~~"));
		return;
    }
	else if (!strcmp(Msg->m_pMessage, "/levels"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Since 40: your shotgun spread growing by 1 bullet per 10 levels"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Since 50: +10 ammo"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Since 100: +10 ammo"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/account"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("--- Account ---"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/register <name> <pass> - Register new account"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/login <name> <pass> - Login to your account"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/logout - Logout from your account"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/password - Change account password"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("--- Account ---"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/shop"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("----- Shop -----"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/range [10 score] - Buy range zombie hammer"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/heart [20 score] - Buy 1 heart if you zombie team"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/jump [3 score] - Buy 1 other jump"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/news"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Added basic ranked system"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/policehelp"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/kick <id> - kick player ids"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/freeze <id> - freeze/unfreeze player"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/idlist - view ids players"));
		return;
	}
	else if(!strcmp(Msg->m_pMessage, "/cmdlist"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ Welcome to cmdlist! ~~~~~~~~"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/register, /login, /logout - account system"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/rules, /help, /info - information"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/stats, /upgr - upgrade system"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/shop - shop score tees"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("/idlist - List IDs players"));	
		if(m_pPlayer->m_AccData.m_PlayerState == 1)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("#This command police group"));
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("#/policehelp - help for police group"));				
		}
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("~~~~~~~~ Welcome to cmdlist! ~~~~~~~~"));
		return;
	}	
	else if(!strcmp(Msg->m_pMessage, "/rules"))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("1. Don't farm XP (freeze acc)"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("2. Don't insult (mute/ban 1-7 day)"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("3. Don't use bots (perma ban)"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("4. Don't self-kill (same as farming)"));
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/heart"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Type '/account' for more information!"));
		if (!GameServer()->GetPlayerChar(m_pPlayer->GetCID())) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use only if you alive!"));
		if (GameServer()->m_World.m_Paused) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please wait for the round to end!"));
		if(m_pPlayer->GetTeam() != TEAM_RED) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Available only zombies!"));
		if (m_pPlayer->m_Score < 20) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient score (20)."));

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Done!"));
		m_pPlayer->m_Score -= 20;
		m_pPlayer->m_ActivesLife = false;
		m_pPlayer->m_LifeActives = false;
		new CLifeHearth(&GameServer()->m_World, vec2(0, 0), m_pPlayer->GetCID());
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/jump"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Use /account for more information"));
		if (!GameServer()->GetPlayerChar(m_pPlayer->GetCID())) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use only if you are alive!"));
		if (GameServer()->m_World.m_Paused) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please wait for the round to end."));
		if (m_pPlayer->m_Score < 3) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient score (3)."));

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Done!"));
		m_pPlayer->m_JumpsShop++;
		m_pPlayer->m_Score -= 3;
		GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_Core.m_Jumps += 1;
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/range"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You are not logged in! Use /account for more information!"));

		if (!GameServer()->GetPlayerChar(m_pPlayer->GetCID())) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use only if you are alive!"));
		if (GameServer()->m_World.m_Paused) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please wait for the round to end."));
		if (m_pPlayer->GetTeam() != TEAM_RED) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Available only to zombies!"));
		if (m_pPlayer->m_Score < 10) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Insufficient score (10)."));

		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Done!"));
		m_pPlayer->m_RangeShop = true;
		m_pPlayer->m_Score -= 10;
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/prefix", 7) && m_pPlayer->m_AccData.m_UserID && 
		(GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()) || m_pPlayer->m_AccData.m_PlayerState == 2 || m_pPlayer->m_AccData.m_UserID == g_Config.m_SvOwnerAccID))
	{
		LastChat();
		char aBuf[24];
		m_pPlayer->m_Prefix ^= true;
		str_format(aBuf, sizeof(aBuf), "Your Prefix %s", m_pPlayer->m_Prefix ? "enable" : "disable");
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/w", 3))
	{
		LastChat();
		int id;
		if (sscanf(Msg->m_pMessage, "/w %d", &id) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Use /w <id> <text>"));
		
		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		char pLsMsg[128];
		str_copy(pLsMsg, Msg->m_pMessage + 5, 128);
		GameServer()->SendPM(m_pPlayer->GetCID(), cid2, pLsMsg);
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/setlvl", 7) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int id, size;
		if ((sscanf(Msg->m_pMessage, "/setlvl %d %d", &id, &size)) != 2)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please use: /setlvl <id> <level>"));

		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[cid2]) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("There is no such player!'"));
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("The player is not logged in account!"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You had to change your level!'")), GameServer()->m_apPlayers[cid2]->m_AccData.m_Level = size, GameServer()->m_apPlayers[cid2]->m_AccData.m_Exp = 0, GameServer()->m_apPlayers[cid2]->m_pAccount->Apply();
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/setmoney", 9) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int id, size;
		if ((sscanf(Msg->m_pMessage, "/setmoney %d %d", &id, &size)) != 2)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please use: /setmoney <id> <money>"));

		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[cid2]) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("There is no such player!'"));
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("The player is not logged in account!"));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You had to change your level!'")), GameServer()->SendChatTarget(cid2, _("Your money changed!'")), GameServer()->m_apPlayers[cid2]->m_AccData.m_Money = size, GameServer()->m_apPlayers[cid2]->m_pAccount->Apply();
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/freeze", 6) && (m_pPlayer->m_AccData.m_PlayerState == 1 || GameServer()->Server()->IsAuthed(m_pPlayer->GetCID())))
	{
		LastChat();
		int id;
		if(sscanf(Msg->m_pMessage, "/freeze %d", &id) != 1) 
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please use: /freeze <id>"));
		
		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if(cid2 == m_pPlayer->GetCID()) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("You can not freeze your account!"));
		if (!GameServer()->m_apPlayers[cid2]) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("There is no such player!'"));
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("The player is not logged in account!"));

		GameServer()->m_apPlayers[cid2]->m_AccData.m_Freeze ^= true;
		GameServer()->m_apPlayers[cid2]->m_pAccount->Apply();
		
		char buf[128];
		str_format(buf, sizeof(buf), "You %s account player '%s'", GameServer()->m_apPlayers[cid2]->m_AccData.m_Freeze ? "Freeze":"Unfreeze", GameServer()->Server()->ClientName(cid2));
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), buf);		
		str_format(buf, sizeof(buf), "Your account %s police '%s'", GameServer()->m_apPlayers[cid2]->m_AccData.m_Freeze ? "Freeze":"Unfreeze", GameServer()->Server()->ClientName(m_pPlayer->GetCID()));
		GameServer()->SendChatTarget(cid2, buf);	
		dbg_msg("police", "Police '%s' freeze acc player '%s' login '%s'", GameServer()->Server()->ClientName(m_pPlayer->GetCID()), GameServer()->Server()->ClientName(cid2), GameServer()->m_apPlayers[cid2]->m_AccData.m_Username);			
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/kick", 5) && m_pPlayer->m_AccData.m_PlayerState == 1)
	{
		LastChat();
		int id;
		if ((sscanf(Msg->m_pMessage, "/kick %d", &id)) != 1)
			return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please use: /kick <id>"));

		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[cid2]) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("There is no such player!'"));

		char buf[64];
		str_format(buf, sizeof(buf), "%s kicked by the Police", GameServer()->Server()->ClientName(cid2));
		GameServer()->SendChatTarget(-1, buf);		
			
		GameServer()->Server()->Kick(cid2, _("You was kicked by the Police!"));			
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/setgroup", 9) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int id, size;
		if (sscanf(Msg->m_pMessage, "/setgroup %d %d", &id, &size) != 2)
		{
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Please use: /setgroup <id> <groupid>")); 
			GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Group ID: 0 - Removed, 1 - Police, 2 - VIP, 3 - Helper")); return;
		}
		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		char gname[4][12] = {"", "police", "vip", "helper"}, aBuf[64];

		if (!GameServer()->m_apPlayers[cid2]) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("There is no such player!"));
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("The player is not logged in account!"));

		switch (size)
		{
			case 0:
				if (GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState)
				{
					str_format(aBuf, sizeof(aBuf), "Removed group %s for player '%s'", gname[GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState], GameServer()->Server()->ClientName(cid2));
					GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
					str_format(aBuf, sizeof(aBuf), "Your group removed %s!", gname[GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState]);
					GameServer()->SendChatTarget(cid2, aBuf);
					GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState = 0;
				}
				else GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("This player already no in group!")); break;
			default:
				if(size > 3 || size < 0) return GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Group ID not found!"));
				GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState = size;
				str_format(aBuf, sizeof(aBuf), "Set group %s for player '%s'", gname[GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState], GameServer()->Server()->ClientName(cid2));
				GameServer()->SendChatTarget(m_pPlayer->GetCID(), aBuf);
				str_format(aBuf, sizeof(aBuf), "Your group set %s!", gname[GameServer()->m_apPlayers[cid2]->m_AccData.m_PlayerState]);
				GameServer()->SendChatTarget(cid2, aBuf); break;
		}
		return;
	}

	if(!strncmp(Msg->m_pMessage, "/", 1))
	{
		LastChat();
		GameServer()->SendChatTarget(m_pPlayer->GetCID(), _("Wrong command. Use /cmdlist"));
	}

}

void CCmd::LastChat()
{
	 m_pPlayer->m_LastChat = GameServer()->Server()->Tick();
}
