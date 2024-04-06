#pragma once

#include "visa.h"

#define TDU_WAVEFORM_SIN	0
#define TDU_WAVEFORM_SQU	1
#define TDU_WAVEFORM_PULS	2

class TekVisaApi
{
public:
	TekVisaApi(void);
	~TekVisaApi(void);

	int TekInit(void);
	void TekDeInit(void);

	int TekSetOutput(int enOutput);
	int TekSetFreq(int freqHz);
	int TekSetWaveform(int waveform);
	int TekSetPulseDuty(int duty);
	int TekSetVoltVpp(float vpp);
	int TekSetImpedance(int imp);
	int TekSetAmDepth(float depth);
	int TekSetAmInternalFreq(int freq);
	int TekSetAmWaveform(int waveform);
	int TekSetAmState(int state);

	int TekStart(void);
	int TekOutWaveForm(int iWaveform, int iDutyCycle, int iImped, int testFreq, float testAmpl);
	int TekEnd(void);
private:
	ViSession mDefRM;
	ViSession mSession;
};
