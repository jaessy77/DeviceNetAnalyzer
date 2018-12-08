#ifndef DEVICENET_ANALYZER_H
#define DEVICENET_ANALYZER_H

#include <Analyzer.h>
#include "DeviceNetAnalyzerResults.h"
#include "DeviceNetSimulationDataGenerator.h"

enum CanBitType
{
	Standard,
	BitStuff
};

class CanMarker
{
public:
	CanMarker(U64 sample, enum CanBitType type)
	{
		mSample = sample;
		mType = type;
	}

	U64 mSample;
	enum CanBitType mType;
};

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
	AnalyzerChannelData* mDeviceNet;
	U32 mSampleRateHz;

	DeviceNetSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitilized;

protected: //analysis functions
	void WaitFor7RecessiveBits();
	void InitSampleOffsets();
	void GetRawFrame();
	void AnalizeRawFrame();
	bool UnstuffRawFrameBit(BitState& result, U64& sample, bool reset = false);
	bool GetFixedFormFrameBit(BitState& result, U64& sample);

protected: //analysis vars:
	//ChunkedArray<ResultBubble>* mFrameBubbles;

	U32 mNumSamplesIn7Bits;
	U32 mRecessiveCount;
	U32 mDominantCount;
	U32 mRawFrameIndex;
	U64 mStartOfFrame;
	U32 mIdentifier;
	U32 mCrcValue;
	bool mAck;

	std::vector<U32> mSampleOffsets;
	std::vector<BitState> mRawBitResults;
	std::vector<BitState> mBitResults;

	std::vector<CanMarker> mCanMarkers;

	std::vector<BitState> mArbitrationField;
	bool mStandardCan;
	bool mRemoteFrame;
	U32 mNumDataBytes;
	std::vector<BitState> mControlField;
	std::vector<BitState> mDataField;
	std::vector<BitState> mCrcFieldWithoutDelimiter;
	BitState mCrcDelimiter;
	std::vector<BitState> mAckField;
	std::vector<BitState> mEndOfFrame;

	U32 mNumRawBits;
	bool mCanError;
	U64 mErrorStartingSample;
	U64 mErrorEndingSample;

};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //DEVICENET_ANALYZER_H
