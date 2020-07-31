
#include <windows.h>
#include <stdio.h>
#include <string>
#pragma comment(lib,"Winmm.lib")

#define DLLEXPORT __declspec(dllexport)
#ifdef __cplusplus
#define DLLEXTERN extern "C"
#else
#define DLLEXTERN extern
#endif





UINT wDeviceID;

UINT __stdcall GetDevId(int a1)
{
	UINT result;
	MCI_OPEN_PARMS mciOpenParms;
	mciOpenParms.lpstrDeviceType = L"waveaudio";
	mciOpenParms.lpstrElementName = L".\\bgm.wav";

	if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_WAIT, (DWORD_PTR)&mciOpenParms))
		result = 0;
	else
		result = mciOpenParms.wDeviceID;
#ifdef _DEBUG
	if (!result)
	{
		printf("InitDevice Failed\n");
	}
#endif
	return result;
}

DLLEXTERN int __stdcall InitDevice(int a1)
{
#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif
	//wDeviceID = GetDevId(a1);
	return 1;
}

UINT __stdcall GetDevIdFromFile(int song_num)
{
	UINT result;
	MCI_OPEN_PARMS mciOpenParms = {0};
	mciOpenParms.lpstrDeviceType = L"waveaudio";
	wchar_t wcNum[40] = {0};
	wsprintf(wcNum, L"bgm/Track%02d.wav", song_num);
	mciOpenParms.lpstrElementName = wcNum;
#ifdef _DEBUG
	printf("%ls\n", wcNum);	
#endif
	if (mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_WAIT, (DWORD_PTR)&mciOpenParms)) {
		result = 0;
	}	
	else {
		result = mciOpenParms.wDeviceID;
	}	
	return result;
}

MCI_STATUS_PARMS _g_status;
int dword_421018; // weak
int dword_42101C; // weak

DLLEXTERN BOOL __stdcall GetPlayStatus(int a1)
{
	BOOL result; // eax

	result = 0;
	dword_42101C = 4;
	if (a1 == 2)
	{	
		mciSendCommand(wDeviceID, MCI_STATUS, 0x102, (DWORD_PTR)&_g_status);
		result = dword_421018 == 526;
	}
	return result;
}

DLLEXTERN int __stdcall PlayMIDI(int a1, int a2, int a3)
{
	return 1;
}


DLLEXTERN int __stdcall StopMIDI(int a1)
{
	return 0;
}






MCI_STATUS_PARMS _g_playcd_status = {0};
MCI_SET_PARMS _g_playcd_set = { 0 };
MCI_PLAY_PARMS _g_playcd_play = { 0 };
int dword_420F9C = 0;


BOOL __stdcall PlayCDSub(DWORD_PTR callback, UINT wDeviceID, int num)//sub_410293
{
	

	// set time format
	_g_playcd_set = { 0 };
	_g_playcd_set.dwTimeFormat = 10;
	mciSendCommand(wDeviceID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&_g_playcd_set);
	
	// status
	_g_playcd_status = { 0 };
	_g_playcd_status.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
	mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM | MCI_WAIT, (DWORD_PTR)&_g_playcd_status);

	_g_playcd_play = { 0 };
	_g_playcd_play.dwCallback = callback;
	//_g_playcd_play.dwFrom = (unsigned char)num;
	//_g_playcd_play.dwTo = (unsigned char)num + 1;


	//int v3; // ecx
	//if (_g_playcd_status.dwReturn == num)
	//	v3 = 0;
	//else
	//	v3 = 8;
	//
	//dword_420F9C = v3 | 5;

	//return mciSendCommand(wDeviceID, MCI_PLAY, MCI_FROM | MCI_TO | MCI_NOTIFY, (DWORD_PTR)&_g_playcd_play);
	return mciSendCommand(wDeviceID, MCI_PLAY, 0, (DWORD_PTR)&_g_playcd_play) == 0;
}

int dword_4200C8 = 0; //callback
int dword_4200C4 = 0; // 1 
int dword_420FAC = 0; // num
UINT devids[256] = {0};
int __stdcall PlayCD(DWORD_PTR callback, unsigned short num, int a3)
{
	if (!devids[num])
	{
		devids[num] = GetDevIdFromFile(num);
	}
	if (devids[num]) {
		wDeviceID = devids[num];
		PlayCDSub(callback, devids[num], num);

#ifdef _DEBUG
		printf("PlayCD : cb - 0x%x, songnum - %d , %d\n", callback, num, a3);
#endif	
	}

	dword_4200C8 = callback;
	dword_4200C4 = 1;
	dword_420FAC = num;
	return 1;
}

int __stdcall RepeatCD(int a1)
{
	PlayCDSub(dword_4200C8, wDeviceID, dword_420FAC);
	return 1;
}

MCI_STATUS_PARMS _g_stopcd_status;
MCI_GENERIC_PARMS _g_stopcd_gp;
BOOL __stdcall sub_410341(int a1, unsigned __int16 a2)
{
	_g_stopcd_status.dwItem = 4;
	mciSendCommand(a2, 0x814u, 0x102u, (DWORD_PTR)&_g_stopcd_status);
	return _g_stopcd_status.dwReturn != 526 && _g_stopcd_status.dwReturn != 529 || !mciSendCommand(a2, MCI_STOP, 2, (DWORD_PTR)&_g_stopcd_gp);
}

int __stdcall StopCD(int a1)
{
	sub_410341(dword_4200C8, wDeviceID);
	--dword_4200C4;
	return 0;
}


int __stdcall PlayPCM(int a1, int a2, int a3)
{
	return 1;
}

int __stdcall StopPCM(int a1)
{
	return 0;
}

MCI_STATUS_PARMS _g_postion;
int GetPositionCD()
{
	_g_postion = { 0 };
	_g_postion.dwItem = 4;
	mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&_g_postion);
	if (_g_postion.dwReturn != 526)
		return -1;
	_g_postion = { 0 };
	_g_postion.dwItem = 2;
	mciSendCommand(wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&_g_postion);
	return _g_postion.dwReturn;
}