#include "stdafx.h"
#include "Wireless_BT_TxTest.h"


// Disable string warning
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)


RF_IMPLEMENT_DYNCREATE(CWireless_BT_TxTest)

CWireless_BT_TxTest::CWireless_BT_TxTest()
	:m_WriteCPKLog(false)
{
}

bool CWireless_BT_TxTest::Run()
{
	m_strMeasured = "";

	char szLower[256];
	memset(szLower, 0, 256);
	sprintf_s(szLower, 255, "%.3f", m_dLower);

	char szUpper[256];
	memset(szUpper, 0, 256);
	sprintf_s(szUpper, 255, "%.3f", m_dUpper);

	char szBand[256];
	memset(szBand, 0, 256);
	strcpy(szBand, "BT");

	// Prepare CPK log
	std::string cpkfile = string(DEFAULT_CPKLOG_PATH) + "Wireless_BT_CPK";
	gCpkRecord.setFileName (cpkfile.c_str());
	gCpkRecord.ResetAll();

	int count = m_CPKHeaderList.size();
	for (int i=0; i < count; ++i)
	{
		gCpkRecord.setHeader (i, m_CPKHeaderList[i].c_str());
	}

	bool bRes = MainFunction();

	if (bRes)
	{
		FactoryLog(true, "BT_TxTest", "Pass", szBand, m_strChannel, szLower, szUpper, m_strMeasured, "dBm", "Wireless BT Tx power PASS");
	}
	else
	{
		FactoryLog(false, "BT_TxTest", FunErr_BT_Test_OutPower_OutRange, szBand, m_strChannel, szLower, szUpper, m_strMeasured, "dBm", m_strMsg);
	}
	SetPICSData(m_strPICSName, m_strMeasured);
	gCpkRecord.assign ("BT_TxPower_VALUE", m_strMeasured); //write BT_TX_VALUE to CPK

	// write result to log file
	TraceLog (MSG_INFO, "Wireless_BT_HEADER: " + gCpkRecord.GetLogHeader());
	TraceLog (MSG_INFO, "Wireless_BT_RESULT: " + gCpkRecord.GetLogString());

	// write result to CPK log
	if (m_WriteCPKLog)
	{
		gCpkRecord.assign ("SN", g_strPicasso);
		gCpkRecord.assign ("Result", ( bRes ? "PASS" : "FAIL"));

		SYSTEMTIME currTime;
		::GetLocalTime(&currTime);
		std::string strTime = MakeSTDstring ("%02d:%02d:%02d", currTime.wHour, currTime.wMinute, currTime.wSecond);
		gCpkRecord.assign ("Time", strTime);

		gCpkRecord.WriteRecordToFile();
	}

	return bRes;
}

bool CWireless_BT_TxTest::MainFunction()
{
	char szTmp[256];

	// Initial tester device
	if (! m_pITesterDevice->Initial())
	{
		m_strMsg = "Fail to initial tester device";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	if (! m_pITesterDevice->Initial_RF_Nonsignaling())
	{
		m_strMsg = "Fail to initial tester device : RF non-signaling";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	m_pITesterDevice->SetMode(1);

	if (! m_pITesterDevice->SetInputPort(1, m_iInputPort))
	{
		m_strMsg = "Fail to set RF input port";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	if (! m_pITesterDevice->RF_InitialAnalyzer())
	{
		m_strMsg = "Fail to initial tester device for RF analyzer";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	if (! m_pITesterDevice->RF_SetAnalyzerFreq(m_dFrequency))
	{
		m_strMsg = "Fail to set RF analyzer frequency to tester device";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	if (! m_pITesterDevice->SetInputLoss(1, m_iInputPort, GetLossByDegree(m_iPowerDegree)))
		return false;
/*
	// Start BT power
	if (! m_pIPhone->Initial_FTD())
	{
		m_strMsg = "Fail to initial phone connection with FTD DLL";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	Sleep(m_iCommandDelay);
*/
	if (! m_pIPhone->BTModuleOnOffSwitch(true, m_i_BTStartDelay))
	{
		m_strMsg = "Fail to turn ON BT function";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}

	//must more than 3s,or BT module can't start up
	//Sleep(m_i_BTStartDelay),move to ftd;

 	if (! m_pIPhone->BTEnterTestMode())
 	{
 		m_strMsg = "Fail to enter BT test mode";
 		TraceLog(MSG_ERROR, m_strMsg);
 		return false;
 	}

	char szBTInput[128] = {0};
	strcpy(szBTInput, m_strBTInput.c_str());
	if (! m_pIPhone->BTStartTxPower(szBTInput))
	{
		m_strMsg = "Fail to start BT TX power";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}
	Sleep(m_iCommandDelay);

	bool bPass = false;
	double dPower;
	std::string strResponse;
	std::vector<std::string> vPowerList;

	double dMax = -99999.0;

	for (int i = 0; i < m_iRetryTimes; ++i)
	{
		if (! m_pITesterDevice->RF_NPowerMeasurement(10, strResponse))
		{
			m_strMsg = "Fail to fetch RF analyzer power";
			TraceLog(MSG_ERROR, m_strMsg);
			return false;
		}

		if (strstr(strResponse.c_str(), "NAN") != NULL)
		{
			sprintf_s(szTmp, 255, "%0d - BT power : NAN", i + 1);
			m_strMsg = szTmp;
			TraceLog(MSG_ERROR, m_strMsg);
			continue;
		}

		STDStringTokenizer(strResponse, ",", vPowerList);
		dPower = atof(vPowerList[5].c_str());

		memset(szTmp, 0, 256);
		sprintf_s(szTmp, 255, "%0d - BT power : %.3f", i + 1, dPower);
		m_strMsg = szTmp;
		TraceLog(MSG_INFO, m_strMsg);

		if (dPower > dMax)
			dMax = dPower;

		if (! InRange(m_dLower, m_dUpper, dMax))
		{
			if (i < m_iRetryTimes - 1)
			{
				m_strMsg = "BT power not in range, try again";
				TraceLog(MSG_ERROR, m_strMsg);
			}
			else
			{
				m_strMsg = "BT power not in range";
				TraceLog(MSG_ERROR, m_strMsg);
			}
		}
		else
		{
			bPass = true;
			break;
		}

		Sleep(500);
	}

	memset(szTmp, 0, 256);
	sprintf_s(szTmp, 255, "%.3f", dMax);
	m_strMeasured = szTmp;

	// Stop BT power
	if (! m_pIPhone->BTModuleOnOffSwitch(false, 0))
	{
		m_strMsg = "Fail to turn OFF BT function";
		TraceLog(MSG_ERROR, m_strMsg);
		return false;
	}


	m_pIPhone->BTReset();

	//if (! m_pIPhone->BTReset())
	//{
	//	m_strMsg = "Fail to turn OFF BT function";
	//	TraceLog(MSG_ERROR, m_strMsg);
	//	return false;
	//}
	//Sleep(m_iCommandDelay);

	return bPass;
}

double CWireless_BT_TxTest::GetLossByDegree(int iDegree)
{
	double dLoss = 0.0;

	switch (iDegree)
	{
	case 0:
		dLoss = GetCableLoss("TX", "WirelessBT", "240");
		break;
	case 1:
		dLoss = GetCableLoss("TX", "WirelessBT", "241");
		break;
	default:
		dLoss = GetCableLoss("TX", "WirelessBT", "242");
	}

	return dLoss;
}


bool CWireless_BT_TxTest::InitData(std::map<std::string, std::string>& paramMap)
{
	std::vector<std::string> vTmp;

	if (paramMap.find("PICSName") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter PICSName for CWireless_BT_TxTest");
		return false;
	}
	m_strPICSName = paramMap["PICSName"];

	if (paramMap.find("CommandDelay") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter CommandDelay for CWireless_BT_TxTest");
		return false;
	}
	m_iCommandDelay = atoi(paramMap["CommandDelay"].c_str());

	if (paramMap.find("BTStartDelay") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter BTStartDelay for CWireless_BT_TxTest");
		return false;
	}
	m_i_BTStartDelay = atoi(paramMap["BTStartDelay"].c_str());

	if (paramMap.find("PowerDegree") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter PowerDegree for CWireless_BT_TxTest");
		return false;
	}
	m_iPowerDegree = atoi(paramMap["PowerDegree"].c_str());

	if (paramMap.find("Frequency") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter Frequency for CWireless_BT_TxTest");
		return false;
	}
	m_dFrequency = atof(paramMap["Frequency"].c_str());
	m_strChannel = paramMap["Frequency"].c_str();

	if (paramMap.find("RetryTimes") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter RetryTimes for CWireless_BT_TxTest");
		return false;
	}
	m_iRetryTimes = atoi(paramMap["RetryTimes"].c_str());

	if (paramMap.find("InputPort") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter InputPort for CWireless_BT_TxTest");
		return false;
	}
	m_iInputPort = atoi(paramMap["InputPort"].c_str());

	if (paramMap.find("BTInput") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter BTInput for CWireless_BT_TxTest");
		return false;
	}
	m_strBTInput = paramMap["BTInput"].c_str();

	if (paramMap.find("Range") == paramMap.end())
	{
		TraceLog(MSG_ERROR, "Fail to find parameter Range for CWireless_BT_TxTest");
		return false;
	}
	STDStringTokenizer(paramMap["Range"], ",", vTmp);
	if ((signed)vTmp.size() < 2)
	{
		TraceLog(MSG_ERROR, "Fail to find parameter Range for CWireless_BT_TxTest");
		return false;
	}
	m_dLower = atof(vTmp[0].c_str());
	m_dUpper = atof(vTmp[1].c_str());

	if (paramMap.find ("WriteCPKLog") != paramMap.end())
		m_WriteCPKLog = paramMap["WriteCPKLog"] != "0";

	ParseCPKItems();

	return true;
}

bool CWireless_BT_TxTest::ParseCPKItems()
{
	m_CPKHeaderList.clear();

	if (m_TestItemXML.Load(m_cstrTestItemFile) != ERROR_SUCCESS)
		return false;

	XMLNode processObjectSet = m_TestItemXML.SearchNode(_T("//Configuration//ProcessObjectSet//WirelessTestObjects//BtCPKcolumn"));
	if (processObjectSet == NULL)
		return false;

	XMLNodeList processObjectsCategoryList = processObjectSet->GetchildNodes();
	if (processObjectsCategoryList == NULL)
		return false;

	int listsize = m_TestItemXML.GetNodeListSize(processObjectsCategoryList);
	for (int j = 0; j < listsize; ++j)
	{
		XMLNode processObject = m_TestItemXML.GetNodeListItem(processObjectsCategoryList, j);
		if (processObject == NULL)
			continue;
			
		// Parse the process object
		CString cstrName = m_TestItemXML.GetNodeAttrValue(processObject, _T("Name"));
		if (cstrName.IsEmpty())
			continue;
		CT2A szName(cstrName);
		std::string strName(szName);

		m_CPKHeaderList.push_back(strName);
	}

	return true;
}