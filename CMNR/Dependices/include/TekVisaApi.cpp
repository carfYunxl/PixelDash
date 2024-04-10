#include "pch.h"
#include "TekVisaApi.h"

#include <conio.h>

TekVisaApi::TekVisaApi(void)
{
	mDefRM = VI_NULL;
	mSession = VI_NULL;
}

TekVisaApi::~TekVisaApi(void)
{
	TekDeInit();
}

int TekVisaApi::TekInit(void){

	ViStatus status;
	ViChar buffer[VI_FIND_BUFLEN];
	ViChar desc[VI_FIND_BUFLEN];
	ViVersion version = 0, impl = 0;
	ViFindList list;
	ViUInt32 retCnt;

	//Open a default Session
	status = viOpenDefaultRM(&mDefRM);
	if (status < VI_SUCCESS) goto INIT_ERROR;

	// Get and print Visa vendor's name, Visa Specification Version, and Implementation Version.
	status = viGetAttribute(mDefRM, VI_ATTR_RSRC_MANF_NAME, buffer);
	if (status < VI_SUCCESS) goto INIT_ERROR;
	status = viGetAttribute(mDefRM, VI_ATTR_RSRC_SPEC_VERSION, &version);
	if (status < VI_SUCCESS) goto INIT_ERROR;
	status = viGetAttribute(mDefRM, VI_ATTR_RSRC_IMPL_VERSION, &impl);
	if (status < VI_SUCCESS) goto INIT_ERROR;
	_cprintf("[TekInit] Manufacturer Name: %s, supports %x spec, %x implimentation version\n", buffer, version, impl);

	//Find USB instruments
	status = viFindRsrc(mDefRM, "USB?*INSTR", &list, &retCnt, desc);
	if (status < VI_SUCCESS) goto INIT_ERROR;
	viClose(list);
	
	//Open USB device
	status = viOpen(mDefRM, desc, VI_NULL, VI_NULL, &mSession);
	if (status < VI_SUCCESS) goto INIT_ERROR;

	// Init to default state
	status = viClear(mSession);
	if (status < VI_SUCCESS) goto INIT_ERROR;

	// Set timeout to 5 seconds
	status = viSetAttribute(mSession, VI_ATTR_TMO_VALUE, 5000);
	if (status < VI_SUCCESS) goto INIT_ERROR;

	status = viSetAttribute(mSession,VI_ATTR_WR_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS);
	status = viSetAttribute(mSession,VI_ATTR_RD_BUF_OPER_MODE, VI_FLUSH_ON_ACCESS);

	// Turn headers off, this makes parsing easier
	status = viPrintf(mSession, "header off\n");
	if (status < VI_SUCCESS) goto INIT_ERROR;

	// Send an ID query.
	status = viWrite(mSession, (ViBuf) "*idn?", 5, &retCnt);
	if (status < VI_SUCCESS) goto INIT_ERROR;

	// Clear the buffer and read the response
	memset(buffer, 0, sizeof(buffer));
	status = viRead(mSession, (ViBuf) buffer, sizeof(buffer), &retCnt);
	if (status < VI_SUCCESS) goto INIT_ERROR;

	// Print the response
	_cprintf("[TekInit] id: %s\n", buffer);

	return 0;

INIT_ERROR:
	// Report error and clean up
	viStatusDesc(mSession, status, buffer);
	_cprintf("[TekInit] %s\n", buffer);
	if (mDefRM != VI_NULL) {
		viClose(mDefRM);
	}
	return -1;
}

void TekVisaApi::TekDeInit()
{
	// Clean up
	if(mSession != VI_NULL) viClose(mSession); 
	if(mDefRM != VI_NULL) viClose(mDefRM);
}

int TekVisaApi::TekSetOutput(int enOutput)
{
	ViStatus	status;
	int output;

	if(mSession == VI_NULL){
		_cprintf("[TekSetOutput] Uninitiated.\n");
		return -1;
	
	}
	//check current state
	status = viQueryf(mSession, "OUTPUT1:STATE?\n", "%d", &output);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetOutput] viQueryf OUTPUT1:STATE error\n");
		return -1;
	}
	if(output == enOutput){
		return 0;
	}

	status = viPrintf(mSession, "OUTPUT1:STATE %d\n",enOutput);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetOutput] viPrintf OUTPUT1:STATE error\n");
		return -1;
	}
	status = viQueryf(mSession, "OUTPUT1:STATE?\n", "%d", &output);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetOutput] viQueryf OUTPUT1:STATE error\n");
		return -1;
	}
	if (output != enOutput)
	{
		_cprintf("[TekSetOutput] Tek Set OUTPUT1:STATE failed\n");
		return -1;
	}
	return 0;
}

int TekVisaApi::TekSetFreq(int freqHz)
{
	ViStatus	status;
	float qfreqHz;
	if(mSession == VI_NULL){
		_cprintf("[TekSetFreq] Uninitiated.\n");
		return -1;
	}
	status = viPrintf(mSession, "FREQUENCY %d\n",freqHz);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetFreq] viPrintf Freq error at freq %dHz\n",freqHz);
		return -1;
	}
	status = viQueryf(mSession, "FREQUENCY?\n", "%e", &qfreqHz);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetFreq] viQueryf Freq error at freq %fHz\n",qfreqHz);
		return -1;
	}
	if (qfreqHz != freqHz)
	{
		//_cprintf("[TekSetFreq] Tek Set Frequency %dHz failed(actual freq= %dHz)\n",freqHz,qfreqHz);
		//return -1;
	}
	return 0;
}


int TekVisaApi::TekSetWaveform(int waveform)
{
	ViStatus	status;
	char qstrWaveform[32];
	char strWaveform[32];
	if(mSession == VI_NULL){
		_cprintf("[TekSetWaveform] Uninitiated.\n");
		return -1;
	}
	switch(waveform)
	{
		case TDU_WAVEFORM_SIN:
			strcpy_s(strWaveform, "SIN");
			break;
		case TDU_WAVEFORM_SQU:
			strcpy_s(strWaveform, "SQU");
			break;
		case TDU_WAVEFORM_PULS:
			strcpy_s(strWaveform, "PULS");
			break;
		default:
			_cprintf("[TekSetWaveform] Input Waveform Type error");
			return -1;
	}
	status = viPrintf(mSession, "FUNC %s\n",strWaveform);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetWaveform] Set Waveform error\n");
		return -1;
	}
	status = viQueryf(mSession, "FUNC?\n", "%s",qstrWaveform);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetWaveform] viQueryf Waveform error\n");
		return -1;
	}
	if (strncmp(strWaveform, qstrWaveform, 3) != 0)
	{
		_cprintf("[TekSetWaveform] Change Waveform error\n");
		return -1;
	}
	return 0;
}


int TekVisaApi::TekSetPulseDuty(int duty)
{
	ViStatus	status;
	float qduty;

	if(mSession == VI_NULL){
		_cprintf("[TekSetPulseDuty] Uninitiated.\n");
		return -1;
	}

	status = viPrintf(mSession, "PULSe:DCYCle %d\n",duty);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetPulseDuty] Set Duty %d error\n",duty);
		return -1;
	}
	status = viQueryf(mSession, "PULSe:DCYCle?\n", "%f", &qduty);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetPulseDuty] viQueryf Duty error\n");
		return -1;
	}
	int delta = 0;
	delta = abs(duty - (int)qduty);
	if(delta > 1 )
	{
		_cprintf("[TekSetPulseDuty] Set duty failed\n");
		return -1;
	}
	return 0;
}

int TekVisaApi::TekSetVoltVpp(float vpp)
{
	ViStatus	status;
	float qvpp;
	viPrintf(mSession, "VOLT:UNIT VPP\n");
	status = viPrintf(mSession, "VOLT:LEV:IMM:AMPL %.1fVPP\n",vpp);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetVoltVpp] Set Vpp %.1f error\n",vpp);
		return -1;
	}
	status = viQueryf(mSession, "VOLT:LEV:IMM:AMPL?\n", "%f", &qvpp);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetVoltVpp] viQueryf Vpp error\n");
		return -1;
	}
	else{
		//_cprintf("[TekSetVoltVpp] Set Vpp as %.1fVPP\n", qvpp);
	}
	//if (vpp != qvpp)
	//{
	//	_cprintf("[TekSetVoltVpp] Set Vpp failed\n");
	//	return -1;
	//}
	return 0;
}


int TekVisaApi::TekSetImpedance(int imp)
{
	ViStatus	status;
	float qImp;

	if(mSession == VI_NULL){
		_cprintf("[TekSetOutput] Uninitiated.\n");
		return -1;
	
	}
	if(imp < 10000){	// imp < 10K ohm
		status = viPrintf(mSession, "OUTPut1:IMPedance %dOHM\n",imp);
		if (status < VI_SUCCESS) 
		{
			_cprintf("[TekSetOutput] viPrintf OUTPUT1:IMPedance error\n");
			return -1;
		}
	}
	else if(imp == 10000){
		status = viPrintf(mSession, "OUTPut1:IMPedance MAXimum\n");
		if (status < VI_SUCCESS) 
		{
			_cprintf("[TekSetOutput] viPrintf OUTPUT1:IMPedance MAXimum error\n");
			return -1;
		}
	}
	else{
		status = viPrintf(mSession, "OUTPut1:IMPedance INFinity\n");
		if (status < VI_SUCCESS) 
		{
			_cprintf("[TekSetOutput] viPrintf OUTPUT1:IMPedance INFinity error\n");
			return -1;
		}
	}

	status = viQueryf(mSession, "OUTPut1:IMPedance?\n", "%e", &qImp);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetOutput] viQueryf OUTPut1:IMPedance? error\n");
		return -1;
	}
	//if (qImp != imp)
	//{
	//	_cprintf("[TekSetOutput] Tek Set OUTPUT1:IMPedance failed\n");
	//	return -1;
	//}
	return 0;
}


int TekVisaApi::TekSetAmDepth(float depth)
{
	ViStatus	status;
	float qdepth;
	status = viPrintf(mSession, "AM:DEPth %.1f\n",depth);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmDepth] Set AM depth %.1f error\n",depth);
		return -1;
	}
	status = viQueryf(mSession, "AM:DEPth?\n", "%f", &qdepth);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmDepth] viQueryf depth error\n");
		return -1;
	}
	else{
		//_cprintf("[TekSetAmDepth] Set AM Depth as %.1f%%\n", qdepth);
	}
	//if (depth != qdepth)
	//{
	//	_cprintf("[TekSetAmDepth] Set  AM Depth failed\n");
	//	return -1;
	//}
	return 0;
}


int TekVisaApi::TekSetAmInternalFreq(int freq){
	ViStatus	status;
	float qfreqHz;
	if(mSession == VI_NULL){
		_cprintf("[TekSetAmInternalFreq] Uninitiated.\n");
		return -1;
	}
	status = viPrintf(mSession, "AM:INTernal:FREQuency %d\n",freq);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmInternalFreq] viPrintf Freq error at freq %dHz\n",freq);
		return -1;
	}
	status = viQueryf(mSession, "AM:INTernal:FREQuency?\n", "%e", &qfreqHz);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmInternalFreq] viQueryf Freq error at freq %fHz\n",qfreqHz);
		return -1;
	}
	if (qfreqHz != freq)
	{
		_cprintf("[TekSetAmInternalFreq] Set Frequency %dHz failed(actual freq= %dHz)\n",freq,qfreqHz);
		return -1;
	}
	return 0;
}

int TekVisaApi::TekSetAmWaveform(int waveform)
{
	ViStatus	status;
	char qstrWaveform[32];
	char strWaveform[32];
	if(mSession == VI_NULL){
		_cprintf("[TekSetAmWaveform] Uninitiated.\n");
		return -1;
	}
	switch(waveform)
	{
		case TDU_WAVEFORM_SIN:
			strcpy_s(strWaveform, "SIN");
			break;
		case TDU_WAVEFORM_SQU:
			strcpy_s(strWaveform, "SQU");
			break;
		case TDU_WAVEFORM_PULS:
		default:
			_cprintf("[TekSetAmWaveform] Input Waveform Type error");
			return -1;
	}
	status = viPrintf(mSession, "AM:INTernal:FUNCtion %s\n",strWaveform);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmWaveform] Set Waveform error\n");
		return -1;
	}
	status = viQueryf(mSession, "AM:INTernal:FUNCtion?\n", "%s",qstrWaveform);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmWaveform] viQueryf Waveform error\n");
		return -1;
	}
	if (strncmp(strWaveform, qstrWaveform, 3) != 0)
	{
		_cprintf("[TekSetAmWaveform] Change Waveform error\n");
		return -1;
	}
	return 0;
}



int TekVisaApi::TekSetAmState(int state)
{
	ViStatus	status;
	char qstrState[32];
	char strState[32];
	if(mSession == VI_NULL){
		_cprintf("[TekSetAmState] Uninitiated.\n");
		return -1;
	}
	switch(state)
	{
		case 0:
			strcpy_s(strState, "OFF");
			break;
		case 1:
			strcpy_s(strState, "ON");
			break;
		default:
			_cprintf("[TekSetAmState] Input AM state error");
			return -1;
	}
	status = viPrintf(mSession, "AM:STATe %s\n",strState);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmState] Set AM State error\n");
		return -1;
	}
	status = viQueryf(mSession, "AM:STATe?\n", "%s",qstrState);
	if (status < VI_SUCCESS) 
	{
		_cprintf("[TekSetAmState] viQueryf error\n");
		return -1;
	}
	
	return 0;
}

int TekVisaApi::TekStart()
{
	int iRet = 0;
	iRet = TekInit();
	if (iRet < 0) {
		TRACE("TekInit was failed.");
		return -1;
	}

	iRet = TekSetOutput(0);
	if (iRet < 0) {
		TRACE("TekSetOutput(0) was failed.");
		return -1;
	}

	iRet = TekSetOutput(1);
	if (iRet < 0) {
		TRACE("TekSetOutput(1) was failed.");
		return -1;
	}
	
	return 0;
}

int TekVisaApi::TekOutWaveForm(int iWaveform, int iDutyCycle, int iImped, int testFreq, float testAmpl)
{
	int iRet = 0;
	//Set TekVISA Waveform
	iRet = TekSetWaveform(iWaveform);
	if (iRet < 0) {
		TRACE(_T("Failed to operate TekVISA TekSetWaveform"));
		return -1;
	}
	//Set TekVISA duty cycle if necessary
	if (2 == iWaveform) {	//pulse waveform
		iRet = TekSetPulseDuty(iDutyCycle);
		if (iRet < 0) {
			TRACE(_T("Failed to operate TekVISA TekSetPulseDuty"));
			return -1;
		}
	}
	//Set TekVISA Frequency
	iRet = TekSetFreq(testFreq);
	if (iRet < 0) {
		TRACE(_T("Failed to operate TekVISA TekSetFreq"));
		return -1;
	}
	//Set TekVISA Impedance
	iRet = TekSetImpedance(iImped);
	if (iRet < 0) {
		TRACE(_T("Failed to operate TekVISA TekSetImpedance"));
		return -1;
	}

	//Set TekVISA Amplitude
	iRet = TekSetVoltVpp(testAmpl);
	if (iRet < 0) {
		TRACE(_T("Failed to operate TekVISA TekSetVoltVpp"));
		return -1;
	}

	return 0;
}

int TekVisaApi::TekEnd()
{
	int iRet = 0;
	//disable output
	iRet = TekSetOutput(0);
	if (iRet < 0) {
		TRACE("TekSetOutput was failed.");
		return -1;
	}
	//Deinit TekVISA
	TekDeInit();
	return 0;
}