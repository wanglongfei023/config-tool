/*==========================================================================
*   Copyright (C) 2019 wang. All rights reserved.
*   
*	Authors:	wanglongfei(395592722@qq.com)
*	Date:		2019/04/08 01:50:41
*	Description：
*
==========================================================================*/

#include <config.h>
#include <string>
using namespace std;
int main()
{
	//Config cfg("../conf/config.ini");
	Config cfg("../conf/config.ini");
	try{
		cout << cfg.get("aa") << endl;
	}catch(string e){
		cout << e << endl;
	}
//	cout << cfg.get("aa") << endl;
//	cout << cfg.get("aa", "BB") << endl;
	return 0;
}
