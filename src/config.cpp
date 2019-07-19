/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/06/08 12:08:38
*	Description：
*
==========================================================================*/

#pragma G++ diagnostic error "-std=c++11"
#include <config.h>

Config::Config(string m_strFileName)
{
	this->m_strFileName = m_strFileName;
	this->m_nLineNum = 0;
	m_bLoadFlag = false;
	create_head();
	pthread_rwlock_init(&m_rwlock, NULL);
}

Config::~Config()
{
	free_join();
	if (this->m_pHead != NULL)
	{
		delete this->m_pHead;
	}
}

void Config::trim(string &str)
{

	if(!str.empty())
	{
		str.erase(0, str.find_first_not_of(" "));
		str.erase(str.find_last_not_of(" ") + 1);
	}
}

void Config::create_head()
{
	NodeInfo *p = new NodeInfo();
	p->strKey = "headstrKey";
	p->strValue = "headstrValue";
	p->pNext = NULL;
	this->m_pHead = p;
}

/*
 *	Description:
 * 		file -> memory.	Add items into list which read from configure file
 *	Mode Example:
 *		key = value
 *		key : value
 *	NB:
 *		The spaces before and after the string are ignored by function `trim`
 */
void Config::load_conf()
{
	free_join();
	this->m_nLineNum = 0;
	string strTmp, strKey, strValue;
	ifstream infile;
	string::size_type index;
	infile.open(this->m_strFileName.c_str());
	if(!infile)
	{
		char errInfo[_MSG_LEN] = {0};
		sprintf(errInfo, "Error: Cannot find the configuration file: `%s`", 
						  this->m_strFileName.c_str()); 
		throw string(errInfo);
	}
	while (!infile.eof())
	{
		strTmp = "";
		getline(infile, strTmp);
		index = strTmp.find("=");
		if (index == string::npos)
		{
			index = strTmp.find(":");
			if (index == string::npos)
			{
				continue;
			}
		}
		strKey = strTmp.substr(0, index);
		strValue = strTmp.substr(index + 1, strTmp.length() - index);
		trim(strKey);
		trim(strValue);
		join_head(strKey,strValue);
	}
	infile.close();
}

/*free all nodes except the node of head*/
void Config::free_join()
{
	NodeInfo* pMove = this->m_pHead->pNext;
	NodeInfo* pCurrent = pMove;
	if (pMove == NULL)
	{
		return;
	}
	while (pCurrent != NULL)
	{
		pMove = pCurrent->pNext;
		delete pCurrent;
		pCurrent = pMove;
	}
	this->m_pHead->pNext = NULL;
}

int Config::get_line_num()
{
	if(!m_bLoadFlag)
	{
		load_conf(); 	
	}
	return this->m_nLineNum;
}

void Config::set(string strKey, string strValue)
{
	pthread_rwlock_wrlock(&m_rwlock);
	if(!m_bLoadFlag)
	{
		load_conf(); 	
	}
	
	join_head(strKey, strValue);
	synchronize_to_file();
	pthread_rwlock_unlock(&m_rwlock);
}

/*
 *	Description:
 *			memory -> file.  Write items into configure file which read from list of memory
 *	Mode Example:
 *			key = value
 *	NB:
 *			It will empty the previous content when write into the configure file
 */
void Config::synchronize_to_file()
{
	if (this->m_pHead == NULL)
	{
		return;
	}
	string strTmp;
	ofstream outfile;
	outfile.open((this->m_strFileName).c_str(), ios::out | ios::trunc);
	if(!outfile)
	{
		char errInfo[_MSG_LEN] = {0};
		sprintf(errInfo, "Error:Cannot find the configuration file: `%s`", 
					      this->m_strFileName.c_str()); 
		throw string(errInfo);
	}

	NodeInfo * pNode = this->m_pHead->pNext;
	while (pNode != NULL)
	{
		strTmp = pNode->strKey + " = " + pNode->strValue;
		outfile << strTmp << endl;
		pNode = pNode->pNext;
	}
	outfile.close();
}

/*Get a specific configuration value by key*/
string Config::get(string strKey, string strDefaultVal)
{
	pthread_rwlock_rdlock(&m_rwlock);
	if(!m_bLoadFlag)
	{
		load_conf(); 	
	}
	if (this->m_pHead == NULL)
	{
		return strDefaultVal;
	}
	NodeInfo *pNode = this->m_pHead->pNext;
	while (pNode != NULL)
	{
		if (pNode->strKey.compare(strKey) == 0)
		{
			strDefaultVal = pNode->strValue;
			break;
		}
		pNode = pNode->pNext;
	}
	pthread_rwlock_unlock(&m_rwlock);
	return strDefaultVal;
}


/*
 *Descripton:  
 * 		Update the value of corresponding item if it's found in list, 
 *		otherwise insert it at the end of the list  
 */
void Config::join_head(string strKey, string strValue)
{
	bool bFindFlag = false;
	if (this->m_pHead == NULL)
	{
		this->create_head();
	}
	NodeInfo * pMove = this->m_pHead;
	NodeInfo * pCurrent = pMove->pNext;
	while (pCurrent != NULL)
	{
		if (pCurrent->strKey.compare(strKey) == 0)
		{
			pCurrent->strValue = strValue;
			bFindFlag = true;
			break;
		}
		pMove = pCurrent;
		pCurrent = pCurrent->pNext;
	}
	if (!bFindFlag)
	{
		NodeInfo *pNewItem = new NodeInfo();
		pNewItem->strKey = strKey;
		pNewItem->strValue = strValue;
		pNewItem->pNext = NULL;
		pMove->pNext = pNewItem;
		this->m_nLineNum ++;
	}
}

void Config::show_all_item()
{
	if (this->m_pHead == NULL)
	{
		return;
	}
	NodeInfo* pNode = this->m_pHead->pNext;
	while (pNode != NULL)
	{
		cout << pNode->strKey << " = " << pNode->strValue << endl;
		pNode = pNode->pNext;
	}
}
