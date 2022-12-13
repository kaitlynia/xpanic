#include <stdio.h>
#include <string.h>

#include <engine/shared/config.h>
#include <engine/server.h>
#include "cmds.h"
#include "account.h"
#include "hearth.h"

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
		if(GameServer()->m_World.m_Paused) return GameServer()->Chat(m_pPlayer->GetCID(), "Please wait for the round to end!");
		
		char Username[256], Password[256];
		if(sscanf(Msg->m_pMessage, "/login %s %s", Username, Password) != 2) 
			return GameServer()->Chat(m_pPlayer->GetCID(), "Use: /login <username> <password>");
		
		if(str_length(Username) > 15 || str_length(Username) < 4 || str_length(Password) > 15 || str_length(Password) < 4) 
			return GameServer()->Chat(m_pPlayer->GetCID(), "Username / Password must contain 4-15 characters");
		
		m_pPlayer->m_pAccount->Login(Username, Password);
		return;
	}
	
	else if(!strncmp(Msg->m_pMessage, "/register", 9))
	{
		LastChat();
		if(GameServer()->m_World.m_Paused)  return GameServer()->Chat(m_pPlayer->GetCID(), "Please wait for the round to end!");
		
		char Username[256], Password[256];
		if(sscanf(Msg->m_pMessage, "/register %s %s", Username, Password) != 2) 
			return GameServer()->Chat(m_pPlayer->GetCID(), "Use /register <username> <password>'");
		
		if(str_length(Username) > 15 || str_length(Username) < 4 || str_length(Password) > 15 || str_length(Password) < 4)
			return GameServer()->Chat(m_pPlayer->GetCID(), "Username and Password must contain 4-15 characters");
		else if (!strcmp(Username, Password))
			return GameServer()->Chat(m_pPlayer->GetCID(), "Username and Password must be different");

		m_pPlayer->m_pAccount->Register(Username, Password);
		return;
	}
	
	else if (!strncmp(Msg->m_pMessage, "/sd", 3) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int size = 0;
		if ((sscanf(Msg->m_pMessage, "/sd %d", &size)) != 1)
			return GameServer()->Chat(m_pPlayer->GetCID(), "Use /sd <idsound>");

		if (m_pPlayer->GetTeam() == TEAM_SPECTATORS || !GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
			return;

		int soundid = clamp(size, 0, 40);
		GameServer()->CreateSound(GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_Pos, soundid);
		return;
	}

	else if(!strcmp(Msg->m_pMessage, "/logout"))
	{
		LastChat();
		if(!m_pPlayer->m_AccData.m_UserID) return GameServer()->Chat(m_pPlayer->GetCID(), "You aren't logged in!");
		if(GameServer()->m_World.m_Paused) return GameServer()->Chat(m_pPlayer->GetCID(), "Please wait for the round to end!");
		if (GameServer()->m_pController->NumZombs() == 1 && m_pPlayer->GetTeam() == TEAM_RED) return GameServer()->Chat(m_pPlayer->GetCID(), "You can't log out as the last zombie!");
		if (GameServer()->m_pController->NumPlayers() < 3 && GameServer()->m_pController->m_Warmup)	return GameServer()->Chat(m_pPlayer->GetCID(), "Wait for the round to start!");

		m_pPlayer->m_pAccount->Apply(), m_pPlayer->m_pAccount->Reset();

		GameServer()->Chat(m_pPlayer->GetCID(), "Logged out.");
		GameServer()->Chat(m_pPlayer->GetCID(), "Use /login <username> <password> to login again");

		if(GameServer()->GetPlayerChar(m_pPlayer->GetCID()) && GameServer()->GetPlayerChar(m_pPlayer->GetCID())->IsAlive())
			GameServer()->GetPlayerChar(m_pPlayer->GetCID())->Die(m_pPlayer->GetCID(), WEAPON_GAME);

		return;
	}	
	
	else if (!strncmp(Msg->m_pMessage, "/upgr", 5))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID) return GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Use /account for more information");

		char supgr[256];
		if (sscanf(Msg->m_pMessage, "/upgr %s", supgr) != 1)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Use /upgr <dmg>, <hp>, <handle>, <ammoregen>, <ammo>, <stats> to upgrade stats");
			return;
		}

		if (!strcmp(supgr, "handle"))
		{
			if (m_pPlayer->m_AccData.m_Handle >= 300)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Maximum handle level already reached (300).");
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient money (1 needed).");

			m_pPlayer->m_AccData.m_Money--;
			m_pPlayer->m_AccData.m_Handle++;
			GameServer()->Chat(m_pPlayer->GetCID(), "New handle: {INT}, Your money: {VAL}", m_pPlayer->m_AccData.m_Handle, m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "dmg"))
		{
			if (m_pPlayer->m_AccData.m_Dmg >= 21)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Maximum damage level already reached (21).");
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient money (1 needed).");

			m_pPlayer->m_AccData.m_Money--;
			m_pPlayer->m_AccData.m_Dmg++;
			GameServer()->Chat(m_pPlayer->GetCID(), "New damage: {INT}, Your money: {VAL}", m_pPlayer->m_AccData.m_Dmg, m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "hp"))
		{
			if (m_pPlayer->m_AccData.m_Health >= 100)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Maximum HP level already reached (100).");
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient money (1 needed).");

			m_pPlayer->m_AccData.m_Money--;
			m_pPlayer->m_AccData.m_Health++;
			GameServer()->Chat(m_pPlayer->GetCID(), "New HP: {INT}, Maximum HP: {VAL}, Your money: {VAL}", m_pPlayer->m_AccData.m_Health, 1000 + m_pPlayer->m_AccData.m_Health * 10 + m_pPlayer->m_AccData.m_Level * 20, m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "ammoregen"))
		{
			if (m_pPlayer->m_AccData.m_Ammoregen >= 60)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Maximum ammo regen level already reached (60).");
			if (m_pPlayer->m_AccData.m_Money <= 0)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient money (1 needed).");

			m_pPlayer->m_AccData.m_Money--;
			m_pPlayer->m_AccData.m_Ammoregen++;
			GameServer()->Chat(m_pPlayer->GetCID(), "New ammo regen: {INT}, Your money: {VAL}", m_pPlayer->m_AccData.m_Ammoregen, m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "ammo"))
		{
			if (m_pPlayer->m_AccData.m_Ammo >= 20)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Maximum ammo level already reached (20).");
			if (m_pPlayer->m_AccData.m_Money < 10)
				return GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient money (10 needed).");

			m_pPlayer->m_AccData.m_Money -= 10, m_pPlayer->m_AccData.m_Ammo++;
			GameServer()->Chat(m_pPlayer->GetCID(), "New ammo: {INT}, Your money: {VAL}", GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_mAmmo + m_pPlayer->m_AccData.m_Ammo, m_pPlayer->m_AccData.m_Money);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "stats"))
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ ! Upgr stats details ! ~~~~~~~~");
			GameServer()->Chat(m_pPlayer->GetCID(), "Damage: {INT}", m_pPlayer->m_AccData.m_Dmg);
			GameServer()->Chat(m_pPlayer->GetCID(), "HP: {VAL}", m_pPlayer->m_AccData.m_Health);
			GameServer()->Chat(m_pPlayer->GetCID(), "Handle: {INT}", m_pPlayer->m_AccData.m_Handle);
			GameServer()->Chat(m_pPlayer->GetCID(), "Ammo regen: {INT}", m_pPlayer->m_AccData.m_Ammoregen);
			GameServer()->Chat(m_pPlayer->GetCID(), "Ammo: {INT}", m_pPlayer->m_AccData.m_Ammo);
			GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ ! Upgr stats details ! ~~~~~~~~");
			return;
		}

		GameServer()->Chat(m_pPlayer->GetCID(), "This type doesn't exist.");
		return;
	}
	else if(!strncmp(Msg->m_pMessage, "/turret", 7))
	{
		LastChat();
		if(!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Use /account for more information!");
			return;
		}

		char supgr[256];
		if (sscanf(Msg->m_pMessage, "/turret %s", supgr) != 1)
			return GameServer()->Chat(m_pPlayer->GetCID(), "Use /turret info");

		if(!strcmp(supgr, "info"))
		{		
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret info - show information");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret help - show help");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret stats - view stats upgrade turrets");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret speed - upgrade turret speed (1 turret money)");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret dmg - upgrade turret damage (1 turret money)");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret ammo - upgrade turret ammo (1 turret money)");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret ammoregen - upgrade turret ammo (1 turret money)");
			GameServer()->Chat(m_pPlayer->GetCID(), "/turret range - upgrade turret range (1 turret money)");
			return;
		}			
		else if(!strcmp(supgr, "help"))
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Turret could be placed via Ghost Emoticon");
			GameServer()->Chat(m_pPlayer->GetCID(), "Turrets can have 3 different weapons");
			GameServer()->Chat(m_pPlayer->GetCID(), "Gun Turret: Shoots once per sec at enemy");
			GameServer()->Chat(m_pPlayer->GetCID(), "Shotgun Turret: Shoots once per 3 sec at enemy");
			GameServer()->Chat(m_pPlayer->GetCID(), "Laser Turret: places a laser each 40 sec if the zombie reach it's line");
			return;
		}
		else if(!strcmp(supgr, "dmg"))
		{
			if (m_pPlayer->m_AccData.m_TurretDmg > 100)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "Maximal turret damage level!");
				return;
			}
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "You have not turret money counts!");
				return;
			}

			m_pPlayer->m_AccData.m_TurretMoney--;
			m_pPlayer->m_AccData.m_TurretDmg++;
			GameServer()->Chat(m_pPlayer->GetCID(), "Your turret's damage is upgraded, now it is: {INT}", m_pPlayer->m_AccData.m_TurretDmg);
			m_pPlayer->m_pAccount->Apply();
			return;
		}			
		else if(!strcmp(supgr, "speed"))
		{
			if (m_pPlayer->m_AccData.m_TurretSpeed >= 500)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "Maximal turret speed level!");
				return;
			}
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "You have not turret money counts!");
				return;
			}

			m_pPlayer->m_AccData.m_TurretMoney--;
			m_pPlayer->m_AccData.m_TurretSpeed++;
			GameServer()->Chat(m_pPlayer->GetCID(), "Your turret's speed is upgraded, now it is: {INT}", m_pPlayer->m_AccData.m_TurretSpeed);
			m_pPlayer->m_pAccount->Apply();
			return;
		}	
		else if (!strcmp(supgr, "ammo"))
		{
			if (m_pPlayer->m_AccData.m_TurretAmmo >= 100)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "Maximal turret ammo level!");
				return;
			}
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "You have not turret money counts!");
				return;
			}

			m_pPlayer->m_AccData.m_TurretMoney--;
			m_pPlayer->m_AccData.m_TurretAmmo++;
			GameServer()->Chat(m_pPlayer->GetCID(), "Your turret's gun is upgraded, now it is: {INT}", m_pPlayer->m_AccData.m_TurretAmmo);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "ammoregen"))
		{
			if (m_pPlayer->m_AccData.m_TurretShotgun >= 75)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "Maximal turret ammoregen level!");
				return;
			}
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "You have not turret money counts!");
				return;
			}

			m_pPlayer->m_AccData.m_TurretMoney--;
			m_pPlayer->m_AccData.m_TurretShotgun++;
			GameServer()->Chat(m_pPlayer->GetCID(), "Your turret's ammoregen is upgraded, now it is: {INT}", m_pPlayer->m_AccData.m_TurretShotgun);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "range"))
		{
			if (m_pPlayer->m_AccData.m_TurretRange >= 200)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "Maximal turret range level!");
				return;
			}
			if (m_pPlayer->m_AccData.m_TurretMoney <= 0)
			{
				GameServer()->Chat(m_pPlayer->GetCID(), "You have not turret money counts!");
				return;
			}

			m_pPlayer->m_AccData.m_TurretMoney--;
			m_pPlayer->m_AccData.m_TurretRange++;
			GameServer()->Chat(m_pPlayer->GetCID(), "Your turret's range is upgraded, now it is: {INT}", m_pPlayer->m_AccData.m_TurretRange);
			m_pPlayer->m_pAccount->Apply();
			return;
		}
		else if (!strcmp(supgr, "stats"))
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "---- Turret ----");
			GameServer()->Chat(m_pPlayer->GetCID(), "Lvl: {INT}", m_pPlayer->m_AccData.m_TurretLevel);
			GameServer()->Chat(m_pPlayer->GetCID(), "Exp: {VAL}", m_pPlayer->m_AccData.m_TurretExp);
			GameServer()->Chat(m_pPlayer->GetCID(), "Money: {VAL}", m_pPlayer->m_AccData.m_TurretMoney);
			GameServer()->Chat(m_pPlayer->GetCID(), "Speed: {INT}", m_pPlayer->m_AccData.m_TurretSpeed);
			GameServer()->Chat(m_pPlayer->GetCID(), "Damage: {INT}", m_pPlayer->m_AccData.m_TurretDmg);
			GameServer()->Chat(m_pPlayer->GetCID(), "Ammo: {INT}", m_pPlayer->m_AccData.m_TurretAmmo);
			GameServer()->Chat(m_pPlayer->GetCID(), "AmmoRegen: {INT}", m_pPlayer->m_AccData.m_TurretShotgun);
			GameServer()->Chat(m_pPlayer->GetCID(), "Range: {INT}", m_pPlayer->m_AccData.m_TurretRange);
			GameServer()->Chat(m_pPlayer->GetCID(), "---- Turret ----");
			return;
		}
		else
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "No such turret upgrade: '{STR}'!", supgr);
			return;
		}
	}
	else if (!str_comp_nocase(Msg->m_pMessage, "/me") || !str_comp_nocase(Msg->m_pMessage, "/status") || !str_comp_nocase(Msg->m_pMessage, "/stats"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}

		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ ! Your account details ! ~~~~~~~~");
		GameServer()->Chat(m_pPlayer->GetCID(), "Login: {STR}", m_pPlayer->m_AccData.m_Username);
		GameServer()->Chat(m_pPlayer->GetCID(), "Password: {STR}", m_pPlayer->m_AccData.m_Password);
		GameServer()->Chat(m_pPlayer->GetCID(), "Mailbox: <no mailbox> (no active)");
		GameServer()->Chat(m_pPlayer->GetCID(), "Level: {INT}", m_pPlayer->m_AccData.m_Level);
		GameServer()->Chat(m_pPlayer->GetCID(), "Exp: {VAL}/{VAL}", m_pPlayer->m_AccData.m_Exp, m_pPlayer->m_AccData.m_Level);
		GameServer()->Chat(m_pPlayer->GetCID(), "Money: {VAL}", m_pPlayer->m_AccData.m_Money);
		GameServer()->Chat(m_pPlayer->GetCID(), "Freeze: {STR}", m_pPlayer->m_AccData.m_Freeze ? "yes" : "no");
		GameServer()->Chat(m_pPlayer->GetCID(), "You can change your password with the command:");
		GameServer()->Chat(m_pPlayer->GetCID(), "/password <new password>");
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ ! Your account details ! ~~~~~~~~");
		return;
	}
	else if(!strncmp(Msg->m_pMessage, "/idlist", 7))
	{
		GameServer()->Chat(m_pPlayer->GetCID(), "--- List IDs ---");
		for(int i = 0; i < MAX_CLIENTS; i++)
		{
			if (!GameServer()->m_apPlayers[i])
				continue;

			GameServer()->Chat(m_pPlayer->GetCID(), "{STR} ID:{INT}", GameServer()->Server()->ClientName(i), i);
		}
		return;
	}
	else if(!strncmp(Msg->m_pMessage, "/password", 9))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}

		char NewPassword[256];
		if (sscanf(Msg->m_pMessage, "/password %s", NewPassword) != 1)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Use \"/password <new password>\"");
			return;
		}

		m_pPlayer->m_pAccount->NewPassword(NewPassword);
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/vip"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "--- ViP ---");
		GameServer()->Chat(m_pPlayer->GetCID(), "ViP: Get 3x Exp & 2x Money & 5 For killing spree");
		GameServer()->Chat(m_pPlayer->GetCID(), "Invis: +1 secounds, InvisCD: 20 secounds");
		GameServer()->Chat(m_pPlayer->GetCID(), "/prefix = +5 ammo + Special Dmg & 5 For killing spree");
		GameServer()->Chat(m_pPlayer->GetCID(), "1 month VIP = 0euros || 3 months VIP = 0euros || LifeTime VIP = 0euros");
		GameServer()->Chat(m_pPlayer->GetCID(), "Payments: Name, name");
		GameServer()->Chat(m_pPlayer->GetCID(), "Skype: nope, Name: nope");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/supervip"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "--- SuperViP ---");
		GameServer()->Chat(m_pPlayer->GetCID(), "SuperVIP: Get 5x Exp & 3x Money & 5 Score4Life & 3 For killing spree");
		GameServer()->Chat(m_pPlayer->GetCID(), "Invis: +3 secounds, InvisCD: 10 secounds");
		GameServer()->Chat(m_pPlayer->GetCID(), "/superprefix = Special Dmg + 15 Ammo, Access to commands:");
		GameServer()->Chat(m_pPlayer->GetCID(), "/resetupgr - Reset your upgr & /supervipmsg - Special messge");
		GameServer()->Chat(m_pPlayer->GetCID(), "1 month SuperVIP = 0euros || 3 months UltimateVIP = 0euros || LifeTime UltimateVIP = 0euros");
		GameServer()->Chat(m_pPlayer->GetCID(), "Payments: Name, name");
		GameServer()->Chat(m_pPlayer->GetCID(), "Skype: nope, Name: nope");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/ultimatevip"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "--- UltimateViP ---");
		GameServer()->Chat(m_pPlayer->GetCID(), "UltimateVIP: Get 7x Exp & 5x Money & 5 Score4Life & 2 For killing spree");
		GameServer()->Chat(m_pPlayer->GetCID(), "Invis: +5 secounds, InvisCD: 5 secounds");
		GameServer()->Chat(m_pPlayer->GetCID(), "/resetupgr - Reset your upgr & /ultimatemsg - Special messge");
		GameServer()->Chat(m_pPlayer->GetCID(), "1 month UltimateVIP = 0euros || 3 months UltimateVIP = 0euros || LifeTime UltimateVIP = 0euros");
		GameServer()->Chat(m_pPlayer->GetCID(), "Payments: Name, name");
		GameServer()->Chat(m_pPlayer->GetCID(), "Skype: nope, Name: nope");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/info"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ Welcome to info! ~~~~~~~~");
		GameServer()->Chat(m_pPlayer->GetCID(), "Owner: {STR} / Skype: {STR}", g_Config.m_SvOwnerName, g_Config.m_SvOwnerSkype);
		GameServer()->Chat(m_pPlayer->GetCID(), "Panic mod by Kurosio, helper: nope");
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ Welcome to info! ~~~~~~~~");
		return;
	}
	else if(!strcmp(Msg->m_pMessage, "/help"))
    {
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ Welcome to help! ~~~~~~~~");
		GameServer()->Chat(m_pPlayer->GetCID(), "/account - information & help account system");
		GameServer()->Chat(m_pPlayer->GetCID(), "/news - for see what news in server");
		GameServer()->Chat(m_pPlayer->GetCID(), "/rules - for read the rules Panic");
		GameServer()->Chat(m_pPlayer->GetCID(), "/pm - send personal message to player");
		GameServer()->Chat(m_pPlayer->GetCID(), "/cmdlist - commands server");
		GameServer()->Chat(m_pPlayer->GetCID(), "/vip - get info vip status");
		GameServer()->Chat(m_pPlayer->GetCID(), "/supervip - get info supervip status");
		GameServer()->Chat(m_pPlayer->GetCID(), "/ultimatevip - get info ultimatevip status");
		GameServer()->Chat(m_pPlayer->GetCID(), "/turret info - info about turrets");
		GameServer()->Chat(m_pPlayer->GetCID(), "/levels - info about level");
		GameServer()->Chat(m_pPlayer->GetCID(), "/shop - shop score tees");
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ Welcome to help! ~~~~~~~~");
		return;
    }
	else if (!strcmp(Msg->m_pMessage, "/levels"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "Since 40: your shotgun spread growing by 1 bullet per 10 levels");
		GameServer()->Chat(m_pPlayer->GetCID(), "Since 50: +10 ammo");
		GameServer()->Chat(m_pPlayer->GetCID(), "Since 100: +10 ammo");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/account"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "--- Account ---");
		GameServer()->Chat(m_pPlayer->GetCID(), "/register <name> <pass> - Register new account");
		GameServer()->Chat(m_pPlayer->GetCID(), "/login <name> <pass> - Login to your account");
		GameServer()->Chat(m_pPlayer->GetCID(), "/logout - Logout from your account");
		GameServer()->Chat(m_pPlayer->GetCID(), "/password - Change account password");
		GameServer()->Chat(m_pPlayer->GetCID(), "--- Account ---");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/shop"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "----- Shop -----");
		GameServer()->Chat(m_pPlayer->GetCID(), "/range [10 score] - Buy range zombie hammer");
		GameServer()->Chat(m_pPlayer->GetCID(), "/heart [20 score] - Buy 1 heart if you zombie team");
		GameServer()->Chat(m_pPlayer->GetCID(), "/jump [3 score] - Buy 1 other jump");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/news"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "Added basic ranked system");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/policehelp"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "/kick <id> - kick player ids");
		GameServer()->Chat(m_pPlayer->GetCID(), "/freeze <id> - freeze/unfreeze player");
		GameServer()->Chat(m_pPlayer->GetCID(), "/idlist - view ids players");
		return;
	}
	else if(!strcmp(Msg->m_pMessage, "/cmdlist"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ Welcome to cmdlist! ~~~~~~~~");
		GameServer()->Chat(m_pPlayer->GetCID(), "/register, /login, /logout - account system");
		GameServer()->Chat(m_pPlayer->GetCID(), "/rules, /help, /info - information");
		GameServer()->Chat(m_pPlayer->GetCID(), "/stats, /upgr - upgrade system");
		GameServer()->Chat(m_pPlayer->GetCID(), "/shop - shop score tees");
		GameServer()->Chat(m_pPlayer->GetCID(), "/idlist - List IDs players");
		if(m_pPlayer->m_AccData.m_PlayerState == PlayerGroup::POLICE)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "#This command police group");
			GameServer()->Chat(m_pPlayer->GetCID(), "#/policehelp - help for police group");
		}
		GameServer()->Chat(m_pPlayer->GetCID(), "~~~~~~~~ Welcome to cmdlist! ~~~~~~~~");
		return;
	}	
	else if(!strcmp(Msg->m_pMessage, "/rules"))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "1. Don't farm XP (freeze acc)");
		GameServer()->Chat(m_pPlayer->GetCID(), "2. Don't insult (mute/ban 1-7 day)");
		GameServer()->Chat(m_pPlayer->GetCID(), "3. Don't use bots (perma ban)");
		GameServer()->Chat(m_pPlayer->GetCID(), "4. Don't self-kill (same as farming)");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/heart"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Type '/account' for more information!");
			return;
		}
		if (!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Use only if you alive!");
			return;
		}
		if (GameServer()->m_World.m_Paused)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please wait for the round to end!");
			return;
		}
		if(m_pPlayer->GetTeam() != TEAM_RED)
		{
			 GameServer()->Chat(m_pPlayer->GetCID(), "Available only zombies!");
			 return;
		}
		if (m_pPlayer->m_Score < 20)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient score (20).");
			return;
		}

		m_pPlayer->m_Score -= 20;
		m_pPlayer->m_ActivesLife = false;
		m_pPlayer->m_LifeActives = false;
		new CLifeHearth(&GameServer()->m_World, vec2(0, 0), m_pPlayer->GetCID());
		GameServer()->Chat(m_pPlayer->GetCID(), "Done!");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/jump"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Use /account for more information");
			return;
		}
		if (!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Use only if you are alive!"); \
			return;
		}
		if (GameServer()->m_World.m_Paused)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please wait for the round to end.");
			return;
		}
		if (m_pPlayer->m_Score < 3)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient score (3).");
			return;
		}

		m_pPlayer->m_JumpsShop++;
		m_pPlayer->m_Score -= 3;
		GameServer()->GetPlayerChar(m_pPlayer->GetCID())->m_Core.m_Jumps += 1;
		GameServer()->Chat(m_pPlayer->GetCID(), "Done!");
		return;
	}
	else if (!strcmp(Msg->m_pMessage, "/range"))
	{
		LastChat();
		if (!m_pPlayer->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You are not logged in! Use /account for more information!");
			return;
		}
		if (!GameServer()->GetPlayerChar(m_pPlayer->GetCID()))
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Use only if you are alive!");
			return;
		}
		if (GameServer()->m_World.m_Paused)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please wait for the round to end.");
			return;
		}
		if (m_pPlayer->GetTeam() != TEAM_RED)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Available only to zombies!");
			return;
		}
		if (m_pPlayer->m_Score < 10)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Insufficient score (10).");
			return;
		}

		m_pPlayer->m_RangeShop = true;
		m_pPlayer->m_Score -= 10;
		GameServer()->Chat(m_pPlayer->GetCID(), "Done!");
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/prefix", 7) && m_pPlayer->m_AccData.m_UserID && 
		(GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()) || m_pPlayer->m_AccData.m_PlayerState == PlayerGroup::VIP || m_pPlayer->m_AccData.m_UserID == g_Config.m_SvOwnerAccID))
	{
		LastChat();
		m_pPlayer->m_Prefix ^= true;
		GameServer()->Chat(m_pPlayer->GetCID(), "Your Prefix {STR}", m_pPlayer->m_Prefix ? "enable" : "disable");
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/w", 3))
	{
		LastChat();
		int id;
		if (sscanf(Msg->m_pMessage, "/w %d", &id) != 1)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Use /w <id> <text>");
			return;
		}

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
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please use: /setlvl <id> <level>");
			return;
		}

		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[cid2])
		{
			 GameServer()->Chat(m_pPlayer->GetCID(), "There is no such player!'");
			 return;
		}
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "The player is not logged in account!");
			return;
		}

		GameServer()->m_apPlayers[cid2]->m_AccData.m_Level = size;
		GameServer()->m_apPlayers[cid2]->m_AccData.m_Exp = 0;
		GameServer()->m_apPlayers[cid2]->m_pAccount->Apply();
		GameServer()->Chat(m_pPlayer->GetCID(), "You had to change your level!'");
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/setmoney", 9) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int id, size;
		if ((sscanf(Msg->m_pMessage, "/setmoney %d %d", &id, &size)) != 2)
		{
			 GameServer()->Chat(m_pPlayer->GetCID(), "Please use: /setmoney <id> <money>");
			 return;
		}

		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[cid2])
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "There is no such player!'");
			return;
		}
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID)
		{
			 GameServer()->Chat(m_pPlayer->GetCID(), "The player is not logged in account!");
			 return;
		}

		GameServer()->m_apPlayers[cid2]->m_AccData.m_Money = size;
		GameServer()->m_apPlayers[cid2]->m_pAccount->Apply();
		GameServer()->Chat(m_pPlayer->GetCID(), "You had to change your level!'");
		GameServer()->Chat(cid2, "Your money changed!'");
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/freeze", 6) && (m_pPlayer->m_AccData.m_PlayerState == PlayerGroup::POLICE || GameServer()->Server()->IsAuthed(m_pPlayer->GetCID())))
	{
		LastChat();
		int id;
		if(sscanf(Msg->m_pMessage, "/freeze %d", &id) != 1)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please use: /freeze <id>");
			return;
		}
		
		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if(cid2 == m_pPlayer->GetCID())
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "You can not freeze your account!");
			return;
		}
		if (!GameServer()->m_apPlayers[cid2])
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "There is no such player!'");
			return;
		}
		if (!GameServer()->m_apPlayers[cid2]->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "The player is not logged in account!");
			return;
		}

		GameServer()->m_apPlayers[cid2]->m_AccData.m_Freeze ^= true;
		GameServer()->m_apPlayers[cid2]->m_pAccount->Apply();
		
		GameServer()->Chat(m_pPlayer->GetCID(), "You {STR} account player '{STR}'", GameServer()->m_apPlayers[cid2]->m_AccData.m_Freeze ? "Freeze" : "Unfreeze", GameServer()->Server()->ClientName(cid2));
		GameServer()->Chat(cid2, "Your account {STR} police '{STR}'", GameServer()->m_apPlayers[cid2]->m_AccData.m_Freeze ? "Freeze" : "Unfreeze", GameServer()->Server()->ClientName(m_pPlayer->GetCID()));
		dbg_msg("police", "Police '%s' freeze acc player '%s' login '%s'", GameServer()->Server()->ClientName(m_pPlayer->GetCID()), GameServer()->Server()->ClientName(cid2), GameServer()->m_apPlayers[cid2]->m_AccData.m_Username);			
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/kick", 5) && m_pPlayer->m_AccData.m_PlayerState == PlayerGroup::POLICE)
	{
		LastChat();
		int id;
		if ((sscanf(Msg->m_pMessage, "/kick %d", &id)) != 1)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please use: /kick <id>");
			return;
		}

		int cid2 = clamp(id, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[cid2])
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "There is no such player!'");
			return;
		}

		GameServer()->Chat(-1, "{STR} kicked by the Police", GameServer()->Server()->ClientName(cid2));
		GameServer()->Server()->Kick(cid2, "You was kicked by the Police!");			
		return;
	}
	else if (!strncmp(Msg->m_pMessage, "/setgroup", 9) && GameServer()->Server()->IsAuthed(m_pPlayer->GetCID()))
	{
		LastChat();
		int scnf_ID, scnf_Value;
		if (sscanf(Msg->m_pMessage, "/setgroup %d %d", &scnf_ID, &scnf_Value) != 2)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Please use: /setgroup <id> <groupid>");
			GameServer()->Chat(m_pPlayer->GetCID(), "Group ID: 0 - Removed, 1 - Police, 2 - VIP, 3 - Helper"); return;
		}

		scnf_ID = clamp(scnf_ID, 0, (int)MAX_CLIENTS - 1);
		if (!GameServer()->m_apPlayers[scnf_ID])
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "There is no such player!");
			return;
		}

		if (!GameServer()->m_apPlayers[scnf_ID]->m_AccData.m_UserID)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "The player is not logged in account!");
			return;
		}

		if ((PlayerGroup)scnf_Value >= PlayerGroup::NUM || (PlayerGroup)scnf_Value < PlayerGroup::NONE)
		{
			GameServer()->Chat(m_pPlayer->GetCID(), "Group ID not found!");
			return;
		}

		const char* pGroupName[4] = { "none", "police", "vip", "helper" };
		GameServer()->Chat(m_pPlayer->GetCID(), "Set group {STR} for player '{STR}'", pGroupName[scnf_Value], GameServer()->Server()->ClientName(scnf_ID));
		GameServer()->Chat(scnf_ID, "Your group set {STR}!", pGroupName[scnf_Value]);
		GameServer()->m_apPlayers[scnf_ID]->m_AccData.m_PlayerState = (PlayerGroup)scnf_Value;
	}

	if(!strncmp(Msg->m_pMessage, "/", 1))
	{
		LastChat();
		GameServer()->Chat(m_pPlayer->GetCID(), "Wrong command. Use /cmdlist");
	}

}

void CCmd::LastChat()
{
	 m_pPlayer->m_LastChat = GameServer()->Server()->Tick();
}
