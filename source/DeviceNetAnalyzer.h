#ifndef DEVICENET_ANALYZER_H
#define DEVICENET_ANALYZER_H

#include <Analyzer.h>
#include "DeviceNetAnalyzerResults.h"
#include "DeviceNetSimulationDataGenerator.h"

class DeviceNetAnalyzerSettings;
class ANALYZER_EXPORT DeviceNetAnalyzer : public Analyzer2
{
public:
	DeviceNetAnalyzer();
	virtual ~DeviceNetAnalyzer();

	virtual void SetupResults();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::auto_ptr< DeviceNetAnalyzerSettings > mSettings;
	std::auto_ptr< DeviceNetAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	DeviceNetSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U32 mStartOfStopBitOffset;
	U32 mEndOfStopBitOffset;
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //DEVICENET_ANALYZER_H
