﻿/*********************************************************************************
 * COPYRIGHT Qisda WR24 2012
 *
 * Description: .h file of AutoJackDetect object.
 * FileName   : JackDetectTest.cpp
 * Version    : $Revision:   1.0  $
 * Author     : Evonne.Hsieh
 * Time       : 2012/04/12
 * History    :
 *  NO      Date        Updated by					Content of change
 *  1    2012/04/12    Evonne.Hsieh            Create for Detroit project MMI test
 ********************************************************************************/

#include "stdafx.h"
#include "AutoJackDetectTest.h"


RF_IMPLEMENT_DYNCREATE(AutoJackDetect)


bool AutoJackDetect::PreRun()
{
	m_strResult = "-";
	m_strItemCode = "-";
	m_strErrorCode = FunErr_Detect_Headset_Fail;
	m_strBand = "-";
	m_strChannel = "-";
	m_strUpper = "-";
	m_strLower = "-";
	m_strMeasured = "";
	m_strUnit = "-";
	m_strMessage = "";

	return true;
}

bool AutoJackDetect::Run()
{
	return CheckJackType();
}

bool AutoJackDetect::PostRun()
{
	return true;
}

bool AutoJackDetect::InitData(std::map<std::string, std::string>& paramMap)
{
	/* ID */
	std::string str_JackType;
	std::string str_JackPrompt;
	m_i_Type = -1;
	m_needPrompt = false;

	if (paramMap.find(std::string("JackType")) != paramMap.end())
	{
		str_JackType = paramMap[std::string("JackType")];
		m_i_Type = CStr::StrToInt(str_JackType);
	}
	else
	{
		TraceLog(MSG_INFO, _T("Failed to Get JackType for class 'AutoJackDetect'"));
		return false;
	}
	if (paramMap.find(std::string("Prompt")) != paramMap.end())
	{
		str_JackPrompt = paramMap[std::string("Prompt")];
		m_needPrompt = CStr::StrToBool(str_JackPrompt);
	}
	else
	{
		TraceLog(MSG_INFO, _T("Failed to Get Prompt for class 'AutoJackDetect'"));
		return false;
	}

	return true;
}

bool AutoJackDetect::CheckJackType()
{
	if (m_needPrompt) {
		/* Init dialog response flag to false */
		m_b_DialogResponse = false;
		m_b_waitFlag = false;
		/* Notify UI to show message dialog */
		std::string msg = CW2A(L"请插上耳机");
		ShowDialog(msg.c_str());
	}
	/* read LSensor ID from mobile */
	bool b_res = true;
	std::string str_msg;
	char sz_JackType[FTD_BUF_SIZE] = {0};
	if (!m_pIPhone->FTD_AUdetectHeadset(m_nFtdPort, m_nFtdTimeOut, sz_JackType))
	{
		str_msg = _T("Read JackType from mobile fail");
		TraceLog(MSG_INFO, str_msg);
		b_res = false;
	}
	/* Compare LSensor ID with config file */
	if (b_res)
	{
		m_strMeasured = sz_JackType;
		if(CStr::StrToInt(m_strMeasured) == m_i_Type)
		{
			str_msg = _T("Read jack type pass! type = ") + m_strMeasured;
			TraceLog(MSG_INFO, str_msg);
		}
		else
		{
			str_msg = _T("Jack type is not matched. Moble:") + m_strMeasured + _T("-- Config File:") + CStr::IntToStr(m_i_Type);
			TraceLog(MSG_INFO, str_msg);
			b_res = false;
		}
	}

	/* test result */
	if(b_res)
	{
		m_strResult = "PASS";
		m_strErrorCode = "-";
		m_strMessage = str_msg;
		TraceLog(MSG_INFO, "Check JackType ID PASS!");
		SetPICSData("JackType_ID_CHECK", "PASS");
	}
	else
	{
		m_strResult = "FAIL";
		m_strMessage = str_msg;
		TraceLog(MSG_INFO, "Check JackType ID FAIL!");
		SetPICSData("JackType_ID_CHECK", "FAIL");
	}
	FactoryLog();
	return b_res;
}
