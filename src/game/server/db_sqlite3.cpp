/* (c) GutZuFusss. See licence.txt in the root of the distribution for more information.     */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "db_sqlite3.h"
#include "gamecontext.h"

bool CQuery::Next()
{
	/*CALL_STACK_ADD();*/

    int Ret = sqlite3_step(m_pStatement);
    return Ret == SQLITE_ROW;
}
void CQuery::Query(CSql *pDatabase, char *pQuery)
{
	/*CALL_STACK_ADD();*/

    m_pDatabase = pDatabase;
    m_pDatabase->Query(this, pQuery);
}
void CQuery::OnData()
{
	/*CALL_STACK_ADD();*/

    Next();
}
int CQuery::GetID(const char *pName)
{
	/*CALL_STACK_ADD();*/

    for (int i = 0; i < GetColumnCount(); i++)
    {
        if (str_comp(GetName(i), pName) == 0)
            return i;
    }
    return -1;
}

CQuery::~CQuery()
{
}

void CSql::WorkerThread()
{
	/*CALL_STACK_ADD();*/

    while(m_Running)
    {
        lock_wait(m_Lock); //lock queue
        if (m_lpQueries.size() > 0)
        {
            CQuery *pQuery = m_lpQueries.front();
            m_lpQueries.pop();
            lock_unlock(m_Lock); //unlock queue

            int Ret;
            Ret = sqlite3_prepare_v2(m_pDB, pQuery->m_Query.c_str(), -1, &pQuery->m_pStatement, 0);
            if (Ret == SQLITE_OK)
            {
                if (!m_Running) //last check
                    break;
                pQuery->OnData();

                sqlite3_finalize(pQuery->m_pStatement);
            }
            else
                dbg_msg("SQLite", "%s", sqlite3_errmsg(m_pDB));

            delete pQuery;
        }
        else
        {
            thread_sleep(100);
            lock_unlock(m_Lock); //unlock queue
        }

        thread_sleep(10);
    }
}

void CSql::InitWorker(void *pUser)
{
	/*CALL_STACK_ADD();*/

    CSql *pSelf = (CSql *)pUser;
    pSelf->WorkerThread();
}

CQuery *CSql::Query(CQuery *pQuery, std::string QueryString)
{
	/*CALL_STACK_ADD();*/

    pQuery->m_Query = QueryString;


    lock_wait(m_Lock);
    m_lpQueries.push(pQuery);
    lock_unlock(m_Lock);

    return pQuery;
}

CSql::CSql(class CGameContext *pGameServer)
{
    m_pGameServer = pGameServer;
	/*CALL_STACK_ADD();*/

    int rc = sqlite3_open("Accounts.db", &m_pDB);
    if (rc)
    {
        dbg_msg("SQLite", "can't open database");
        sqlite3_close(m_pDB);
    }

	char *pQuery = (char *)"CREATE TABLE IF NOT EXISTS Accounts (" \
		"ID INTEGER					PRIMARY KEY		AUTOINCREMENT," \
        "Username                   TEXT            NOT NULL," \
        "Password                   TEXT            NOT NULL," \
		"Exp                        INTEGER         NOT NULL        DEFAULT 0," \
        "Level                      INTEGER         NOT NULL        DEFAULT 1," \
        "Money                      INTEGER         NOT NULL        DEFAULT 0," \
        "Dmg                        INTEGER         NOT NULL        DEFAULT 0," \
        "Health                     INTEGER         NOT NULL        DEFAULT 0," \
        "Ammoregen                  INTEGER         NOT NULL        DEFAULT 0," \
        "Handle                     INTEGER         NOT NULL        DEFAULT 0," \
        "Ammo                       INTEGER         NOT NULL        DEFAULT 0," \
        "PlayerState                INTEGER         NOT NULL        DEFAULT 0," \
        "TurretMoney                INTEGER         NOT NULL        DEFAULT 0," \
        "TurretLevel                INTEGER         NOT NULL        DEFAULT 0," \
        "TurretExp                  INTEGER         NOT NULL        DEFAULT 0," \
        "TurretDmg                  INTEGER         NOT NULL        DEFAULT 0," \
        "TurretSpeed                INTEGER         NOT NULL        DEFAULT 0," \
        "TurretAmmo                 INTEGER         NOT NULL        DEFAULT 0," \
        "TurretShotgun              INTEGER         NOT NULL        DEFAULT 0," \
        "TurretRange                INTEGER         NOT NULL        DEFAULT 0," \
        "Freeze                     INTEGER         NOT NULL        DEFAULT 0," \
        "Winner                     INTEGER         NOT NULL        DEFAULT 0," \
		"Luser                      INTEGER         NOT NULL        DEFAULT 0);";

    sqlite3_exec(m_pDB, pQuery, 0, 0, 0);

    m_Lock = lock_create();
    m_Running = true;
    thread_init(InitWorker, this);
}

CSql::~CSql()
{
	/*CALL_STACK_ADD();*/

    m_Running = false;
    lock_wait(m_Lock);
    while (m_lpQueries.size())
    {
        CQuery *pQuery = m_lpQueries.front();
        m_lpQueries.pop();
        delete pQuery;
    }
    lock_unlock(m_Lock);
    lock_destroy(m_Lock);
}

bool CSql::Register(const char *Username, const char *Password, int ClientID)
{
    int rc = sqlite3_open("Accounts.db", &m_pDB);
    if (rc)
    {
        dbg_msg("SQLite", "can't open database");
        sqlite3_close(m_pDB);
        return false;
    }

    char pQuery[555];
    
    str_format(pQuery, sizeof(pQuery), (char *)"INSERT INTO Accounts(" \
		"Username, " \
        "Password) " \
        "VALUES ('%s', '%s');", 
        Username, Password);

    char *pErrorMsg;
    sqlite3_exec(m_pDB, pQuery, NULL, NULL, &pErrorMsg);

    m_Lock = lock_create();
    m_Running = true;
    thread_init(InitWorker, this);

    return true;
}

bool CSql::Login(const char *Username, const char *Password, int ClientID)
{
    int rc = sqlite3_open("Accounts.db", &m_pDB);
    if (rc)
    {
        dbg_msg("SQLite", "can't open database");
        sqlite3_close(m_pDB);
        return false;
    }

    char pQuery[555];
    
    str_format(pQuery, sizeof(pQuery), (char *)"SELECT * FROM Accounts WHERE" \
		"Username='%s' AND " \
        "Password='%s';", 
        Username, Password);

    char *pErrorMsg;
    sqlite3_exec(m_pDB, pQuery, NULL, NULL, &pErrorMsg);

    m_Lock = lock_create();
    m_Running = true;
    thread_init(InitWorker, this);

    return true;
}

void CSql::Apply(const char *Username, const char *Password, int ClientID)
{
    int rc = sqlite3_open("Accounts.db", &m_pDB);
    if (rc)
    {
        dbg_msg("SQLite", "can't open database");
        sqlite3_close(m_pDB);
        return;
    }

    char pQuery[700];
    
    str_format(pQuery, sizeof(pQuery), (char *)"UPDATE Accounts SET" \
        "Exp=%d," \
        "Level=%d," \
        "Money=%d," \
        "Dmg=%d," \
        "Health=%d," \
        "Ammoregen=%d," \
        "Handle=%d," \
        "Ammo=%d," \
        "PlayerState=%d," \
        "TurretMoney=%d," \
        "TurretLevel=%d," \
        "TurretExp=%d," \
        "TurretDmg=%d," \
        "TurretSpeed=%d," \
        "TurretAmmo=%d," \
        "TurretShotgun=%d," \
        "TurretRange=%d," \
        "Freeze=%d," \
        "Winner=%d," \
		"Luser=%d " \
        "WHERE Username='%s';", \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Exp, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Level, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Money, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Dmg, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Health, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Ammoregen, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Handle, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Ammo, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_PlayerState, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretMoney, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretLevel, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretExp, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretDmg, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretSpeed, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretAmmo, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretShotgun, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_TurretRange, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Freeze, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Winner, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Luser, \
        m_pGameServer->m_apPlayers[ClientID]->m_AccData.m_Username);

    char *pErrorMsg;
    sqlite3_exec(m_pDB, pQuery, NULL, NULL, &pErrorMsg);

    m_Lock = lock_create();
    m_Running = true;
    thread_init(InitWorker, this);
}