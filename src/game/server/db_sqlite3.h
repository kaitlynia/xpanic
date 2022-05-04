/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */

#ifndef GAME_SERVER_SQLITE3_H
#define GAME_SERVER_SQLITE3_H
#include "base/system.h"
#include <sqlite3.h>
#include <vector>
#include <queue>
#include <string>
#include <engine/server.h>

class CQuery
{
    friend class CSql;
private:
    std::string m_Query;
    sqlite3_stmt *m_pStatement;
    virtual void OnData();
public:
    bool Next();
    bool Busy();
    int GetColumnCount() { return sqlite3_column_count(m_pStatement); }
    const char *GetName(int i) { return sqlite3_column_name(m_pStatement, i); }
    int GetType(int i) { return sqlite3_column_type(m_pStatement, i); }

    int GetID(const char *pName);
    int GetInt(int i) { return sqlite3_column_int(m_pStatement, i); }
    float GetFloat(int i) { return sqlite3_column_double(m_pStatement, i); }
    const char *GetText(int i) { return (const char *)sqlite3_column_text(m_pStatement, i); }
    const void *GetBlob(int i) { return sqlite3_column_blob(m_pStatement, i); }
    int GetSize(int i) { return sqlite3_column_bytes(m_pStatement, i); }

    class CSql *m_pDatabase;
    void Query(class CSql *pDatabase, char *pQuery);

    virtual ~CQuery();
};

class CSql
{
private:
    void WorkerThread();
    static void InitWorker(void *pSelf);
    LOCK m_Lock;

    std::queue<CQuery *>m_lpQueries;

    bool m_Running;

    sqlite3 *m_pDB;

public:
    CSql();
    ~CSql();
    CQuery *Query(CQuery *pQuery, std::string QueryString);

    bool Register(const char *Username, const char *Password, const char *Language, int ClientID);
    bool Login(const char *Username, const char *Password, int ClientID);
    bool Apply(const char *Username, const char *Password, const char *Language, int AccID, 
                int m_PlayerState, int m_Level, int m_Exp, unsigned int m_Money, int m_Dmg, int m_Health, int m_Ammoregen, int m_Handle, int m_Ammo, unsigned int m_TurretMoney, int m_TurretLevel, int m_TurretExp, int m_TurretDmg, int m_TurretSpeed, int m_TurretAmmo, int m_TurretShotgun, int m_TurretRange, int m_Freeze, int m_Winner, int m_Luser);
    /*struct
	{
		int m_UserID[MAX_CLIENTS];
		char m_Username[MAX_CLIENTS][32];
		char m_Password[MAX_CLIENTS][32];
		int m_PlayerState[MAX_CLIENTS];

		int m_Level[MAX_CLIENTS];
		int m_Exp[MAX_CLIENTS];
		unsigned int m_Money[MAX_CLIENTS];

		int m_Dmg[MAX_CLIENTS];
		int m_Health[MAX_CLIENTS];
		int m_Ammoregen[MAX_CLIENTS];
		int m_Handle[MAX_CLIENTS];
		int m_Ammo[MAX_CLIENTS];

		unsigned int m_TurretMoney[MAX_CLIENTS];
		int m_TurretLevel[MAX_CLIENTS];
		int m_TurretExp[MAX_CLIENTS];
		int m_TurretDmg[MAX_CLIENTS];
		int m_TurretSpeed[MAX_CLIENTS];
		int m_TurretAmmo[MAX_CLIENTS];
		int m_TurretShotgun[MAX_CLIENTS];
		int m_TurretRange[MAX_CLIENTS];
		
		int m_Freeze[MAX_CLIENTS];
		int m_Winner[MAX_CLIENTS];
		int m_Luser[MAX_CLIENTS];
	} m_Tmp;*/
};



#endif
