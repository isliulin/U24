
#pragma once
#include "../../../CommonUtility/BaseObject/ITestItem.h"

//#include "CommDef.h"

/*======================================== Const ==========================*/
#define MAXBLOCK		   4096	        // input buffer and out buffer
#define MAXTIMEOUTS		   2000	        // timeout(ms)
#define EVENTCHAR          0x0d	        // event char
#define XON			       0x11	        // DCB XonChar
#define XOFF			   0x13	        // DCB XoffChar
/*======================================== Const ==========================*/

class CCommunicate
{
public:
	CCommunicate(void);
	~CCommunicate(void);

	int SendCommand(const BYTE* b_command, DWORD dw_writeLen, DWORD dr_readLen);
	//int SendCommand2(const BYTE* b_command, DWORD dw_writeLen, DWORD dr_readLen);
	int SetCOMPort(CString str_COMPort);
	int OpenCOM(void);
	void CloseCOM(void);

	DWORD WriteCOM(const BYTE *b_sendBuffer, DWORD dw_writeLen);
	DWORD ReadCOM(char* b_ReadBuffer, DWORD dw_writeLen);

	bool ConfigureCOMPort(CString strPort);
	//DWORD ReadCOM2(char* b_ReadBuffer, DWORD dw_writeLen );

	CString getErrCode();
	CString getErrMsg();
private:

	int ConfigCOM(void);
	bool SetDCB(HANDLE h_Com);

	//DWORD WriteCOM(const BYTE *b_sendBuffer, DWORD dw_writeLen);
	//DWORD ReadCOM(char* b_ReadBuffer, DWORD dw_writeLen);


private:
	HANDLE         m_h_com;
	HANDLE	       m_h_Com;
	DWORD          m_dw_baud;
	INT            m_i_dataBits;
	INT            m_i_parity;
	INT            m_i_stopBits;
	INT            m_i_flowCtrl;
	OVERLAPPED     m_os_write;
	OVERLAPPED     m_os_read;
	CString        m_str_port;
	BOOL           m_b_connected;

	CString        m_str_errCode;
	CString        m_str_errMsg;
};
