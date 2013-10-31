#pragma once

#include "CLog.h"

#include "CMySQLHandle.h"
#include "CMySQLResult.h"
#include "CMySQLQuery.h"


unordered_map<int, CMySQLHandle *> CMySQLHandle::SQLHandle;
CMySQLHandle *CMySQLHandle::ActiveHandle = nullptr;


CMySQLHandle::CMySQLHandle(int id) : 
	
	//m_QueryCounter(0),
	
	m_MyID(id),
	
	//m_ActiveResult(NULL),
	m_ActiveResultID(0),
	
	m_MainConnection(nullptr)
{
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::CMySQLHandle", "constructor called");
}

CMySQLHandle::~CMySQLHandle() 
{
	//for (unordered_map<int, CMySQLResult*>::iterator it = m_SavedResults.begin(), end = m_SavedResults.end(); it != end; it++)
	//	it->second->Destroy();

	m_MainConnection->Destroy();
	ExecuteOnConnectionPool(&CMySQLConnection::Destroy);

	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::~CMySQLHandle", "deconstructor called");
}

void CMySQLHandle::WaitForQueryExec() 
{
	//while(!CCallback::m_CallbackQueue.empty())
	//	boost::this_thread::sleep(boost::posix_time::milliseconds(5));
}

CMySQLHandle *CMySQLHandle::Create(string host, string user, string pass, string db, size_t port, bool reconnect) 
{
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", "creating new connection..");

	int ID = 1;
	if(SQLHandle.size() > 0) 
	{
		unordered_map<int, CMySQLHandle*>::iterator itHandle = SQLHandle.begin();
		do 
		{
			ID = itHandle->first+1;
			++itHandle;
		} while(SQLHandle.find(ID) != SQLHandle.end());
	}


	CMySQLHandle *Handle = new CMySQLHandle(ID);

	//init connections
	size_t PoolSize = 16;
	Handle->m_MainConnection = CMySQLConnection::Create(host, user, pass, db, port, reconnect);
	for (size_t i = 0; i < PoolSize; ++i)
		Handle->m_ConnectionPool.push_front(std::make_tuple(CMySQLConnection::Create(host, user, pass, db, port, reconnect), false));

	SQLHandle.insert( unordered_map<int, CMySQLHandle*>::value_type(ID, Handle) );
	CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::Create", "connection created with ID = %d", ID);
		
	return Handle;
}

void CMySQLHandle::Destroy() 
{
	SQLHandle.erase(m_MyID);
	delete this;
}

CMySQLConnection *CMySQLHandle::GetFreeQueryConnection()
{
	for (auto &t : m_ConnectionPool)
	{
		bool &State = std::get<1>(t);
		if (State == false) {
			State = true;
			return std::get<0>(t);
		}
	}
	return nullptr;
}

void CMySQLHandle::SetQueryConnectionFree(const CMySQLConnection *connection)
{
	for (auto &t : m_ConnectionPool)
	{
		if (std::get<0>(t) == connection)
		{
			std::get<1>(t) = false;
			break;
		}
	}
}

void CMySQLHandle::ExecuteOnConnectionPool(void(CMySQLConnection::*func)())
{
	for (auto &t : m_ConnectionPool)
	{
		(std::get<0>(t)->*func)();
		
	}

}
/*
void CMySQLHandle::ProcessQueries() 
{
	mysql_thread_init();
	while(m_QueryThreadRunning) 
	{
		CMySQLQuery *query = NULL;
		while(m_QueryQueue.pop(query)) 
		{
			query->Execute();
			m_QueryCounter--;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	mysql_thread_end();
}
*/
/*
int CMySQLHandle::SaveActiveResult() 
{
	if(m_ActiveResult != NULL) 
	{
		if(m_ActiveResultID != 0) //if active cache was already saved
		{
			CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::SaveActiveResult", "active cache was already saved");
			return m_ActiveResultID; //return the ID of already saved cache
		}
		else 
		{
			int ID = 1;
			if(!m_SavedResults.empty()) 
			{
				unordered_map<int, CMySQLResult*>::iterator itHandle = m_SavedResults.begin();
				do 
				{
					ID = itHandle->first+1;
					++itHandle;
				} while(m_SavedResults.find(ID) != m_SavedResults.end());
			}

			m_ActiveResultID = ID;
			m_SavedResults.insert( std::map<int, CMySQLResult*>::value_type(ID, m_ActiveResult) );
			
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SaveActiveResult", "cache saved with ID = %d", ID);
			return ID; 
		}
	}
	
	return 0;
}

bool CMySQLHandle::DeleteSavedResult(int resultid) 
{
	if(resultid > 0) 
	{
		if(m_SavedResults.find(resultid) != m_SavedResults.end()) 
		{
			CMySQLResult *ResultHandle = m_SavedResults.at(resultid);
			if(m_ActiveResult == ResultHandle) 
			{
				m_ActiveResult = NULL;
				m_ActiveResultID = 0;
			}
			//ResultHandle->Destroy();
			m_SavedResults.erase(resultid);
			CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::DeleteSavedResult", "result deleted");
			return true;
		}
	}
	
	CLog::Get()->LogFunction(LOG_WARNING, "CMySQLHandle::DeleteSavedResult", "invalid result ID ('%d')", resultid);
	return false;
}

bool CMySQLHandle::SetActiveResult(int resultid) 
{
	if(resultid > 0) 
	{
		if(m_SavedResults.find(resultid) != m_SavedResults.end()) 
		{
			CMySQLResult *cResult = m_SavedResults.at(resultid);
			if(cResult != NULL) 
			{
				if(m_ActiveResult != NULL)
					if(m_ActiveResultID == 0) //if cache not saved
						//m_ActiveResult->Destroy(); //delete unsaved cache
				
				m_ActiveResult = cResult; //set new active cache
				m_ActiveResultID = resultid; //new active cache was stored previously
				CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "result is now active");
			}
		}
		else
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLHandle::SetActiveResult", "result not found");
	}
	else 
	{
		if(m_ActiveResultID == 0) //if cache not saved
			//m_ActiveResult->Destroy(); //delete unsaved cache
		//m_ActiveResult = NULL;
		m_ActiveResultID = 0;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLHandle::SetActiveResult", "invalid result ID specified, setting active result to zero");
	}
	return true;
}
*/
void CMySQLHandle::ClearAll()
{
	for(unordered_map<int, CMySQLHandle *>::iterator i = SQLHandle.begin(); i != SQLHandle.end(); ++i)
		i->second->Destroy();
	
	SQLHandle.clear();
}

void CMySQLHandle::SetActiveResult(CMySQLResult result)
{
	m_ActiveResult = boost::move(result);
	m_ActiveResultID = 0;
}



CMySQLConnection *CMySQLConnection::Create(string &host, string &user, string &passwd, string &db, unsigned int port, bool auto_reconnect)
{
	return new CMySQLConnection(host, user, passwd, db, port, auto_reconnect);
}

void CMySQLConnection::Destroy()
{
	delete this;
}

void CMySQLConnection::Connect() 
{
	if(m_Connection == NULL) 
	{
		m_Connection = mysql_init(NULL);
		if (m_Connection == NULL)
			CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::Connect", "MySQL initialization failed");
	}

	if (!m_IsConnected && !mysql_real_connect(m_Connection, m_Host.c_str(), m_User.c_str(), m_Passw.c_str(), m_Database.c_str(), m_Port, NULL, NULL)) 
	{
		CLog::Get()->LogFunction(LOG_ERROR, "CMySQLConnection::Connect", "(error #%d) %s", mysql_errno(m_Connection), mysql_error(m_Connection));

		m_IsConnected = false;
	} 
	else 
	{
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Connect", "connection was successful");

		my_bool reconnect = m_AutoReconnect;
		mysql_options(m_Connection, MYSQL_OPT_RECONNECT, &reconnect);
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Connect", "auto-reconnect has been %s", m_AutoReconnect == true ? "enabled" : "disabled");
		
		m_IsConnected = true;
	}
}

void CMySQLConnection::Disconnect() 
{
	if (m_Connection == NULL)
		CLog::Get()->LogFunction(LOG_WARNING, "CMySQLConnection::Disconnect", "no connection available");
	else 
	{
		mysql_close(m_Connection);
		m_Connection = NULL;
		m_IsConnected = false;
		CLog::Get()->LogFunction(LOG_DEBUG, "CMySQLConnection::Disconnect", "connection was closed");
	}
}

void CMySQLConnection::EscapeString(const char *src, string &dest)
{
	if(src != NULL && m_IsConnected) 
	{
		size_t SrcLen = strlen(src);
		char *tmpEscapedStr = (char *)malloc((SrcLen*2 + 1) * sizeof(char));

		size_t EscapedLen = mysql_real_escape_string(m_Connection, tmpEscapedStr, src, SrcLen);
		dest.assign(tmpEscapedStr);

		free(tmpEscapedStr);
	}
}
