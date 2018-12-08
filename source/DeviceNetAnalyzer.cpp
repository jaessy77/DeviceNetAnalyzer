#include "DeviceNetAnalyzer.h"
#include "DeviceNetAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

#include "DeviceNetProtocol.h"

DeviceNetAnalyzer::DeviceNetAnalyzer()
:	Analyzer2(),  
	mSettings( new DeviceNetAnalyzerSettings() ),
	mSimulationInitilized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

DeviceNetAnalyzer::~DeviceNetAnalyzer()
{
	KillThread();
}

void DeviceNetAnalyzer::SetupResults()
{
	mResults.reset( new DeviceNetAnalyzerResults( this, mSettings.get() ) );
	SetAnalyzerResults( mResults.get() );
	mResults->AddChannelBubblesWillAppearOn( mSettings->mDeviceNetChannel );
}

void DeviceNetAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	mDeviceNet = GetAnalyzerChannelData( mSettings->mDeviceNetChannel );

	InitSampleOffsets();

	if( mDeviceNet->GetBitState() == BIT_LOW )
		mDeviceNet->AdvanceToNextEdge();

	U32 samples_per_bit = mSampleRateHz / mSettings->mBitRate;
	U32 samples_to_first_center_of_first_data_bit = U32( 1.5 * double( mSampleRateHz ) / double( mSettings->mBitRate ) );

	for( ; ; )
	{
		U32 data = 0;
		U32 mask = 1 << (LENGTH_ARBITRATION_FIELD - 1);
		
		mDeviceNet->AdvanceToNextEdge(); //falling edge -- beginning of the start bit

		U64 starting_sample = mDeviceNet->GetSampleNumber();

		mDeviceNet->Advance( samples_to_first_center_of_first_data_bit );

		for( U32 i=0; i< LENGTH_ARBITRATION_FIELD; i++ )
		{
			//let's put a dot exactly where we sample this bit:
			mResults->AddMarker( mDeviceNet->GetSampleNumber(), AnalyzerResults::Dot, mSettings->mDeviceNetChannel );

			if( mDeviceNet->GetBitState() == BIT_HIGH )
				data |= mask;

			mDeviceNet->Advance( samples_per_bit );

			mask = mask >> 1;
		}


		//we have a byte to save. 
		Frame frame;
		frame.mData1 = data;
		frame.mFlags = 0;
		frame.mStartingSampleInclusive = starting_sample;
		frame.mEndingSampleInclusive = mDeviceNet->GetSampleNumber();

		mResults->AddFrame( frame );
		mResults->CommitResults();
		ReportProgress( frame.mEndingSampleInclusive );
	}
}

bool DeviceNetAnalyzer::NeedsRerun()
{
	return false;
}

U32 DeviceNetAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitilized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitilized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 DeviceNetAnalyzer::GetMinimumSampleRateHz()
{
	return mSettings->mBitRate * 4;
}

const char* DeviceNetAnalyzer::GetAnalyzerName() const
{
	return "DeviceNet";
}

const char* GetAnalyzerName()
{
	return "DeviceNet";
}

Analyzer* CreateAnalyzer()
{
	return new DeviceNetAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}

void DeviceNetAnalyzer::InitSampleOffsets()
{
	mSampleOffsets.resize(256);

	double samples_per_bit = double(mSampleRateHz) / double(mSettings->mBitRate);
	double samples_behind = 0.0;

	U32 increment = U32((samples_per_bit * .5) + samples_behind);
	samples_behind = (samples_per_bit * .5) + samples_behind - double(increment);

	mSampleOffsets[0] = increment;
	U32 current_offset = increment;

	for (U32 i = 1; i < 256; i++)
	{
		U32 increment = U32(samples_per_bit + samples_behind);
		samples_behind = samples_per_bit + samples_behind - double(increment);
		current_offset += increment;
		mSampleOffsets[i] = current_offset;
	}

	mNumSamplesIn7Bits = U32(samples_per_bit * 7.0);
}

void DeviceNetAnalyzer::WaitFor7RecessiveBits()
{
	if (mDeviceNet->GetBitState() == mSettings->Dominant())
		mDeviceNet->AdvanceToNextEdge();

	for (; ; )
	{
		if (mDeviceNet->WouldAdvancingCauseTransition(mNumSamplesIn7Bits) == false)
			return;

		mDeviceNet->AdvanceToNextEdge();
		mDeviceNet->AdvanceToNextEdge();
	}
}

void DeviceNetAnalyzer::GetRawFrame()
{
	mCanError = false;
	mRecessiveCount = 0;
	mDominantCount = 0;
	mRawBitResults.clear();

	if (mDeviceNet->GetBitState() != mSettings->Dominant())
		AnalyzerHelpers::Assert("GetFrameOrError assumes we start DOMINANT");

	mStartOfFrame = mDeviceNet->GetSampleNumber();

	U32 i = 0;
	//what we're going to do now is capture a sequence up until we get 7 recessive bits in a row.
	for (; ; )
	{
		if (i > 255)
		{
			//we are in garbage data most likely, lets get out of here.
			return;
		}

		mDeviceNet->AdvanceToAbsPosition(mStartOfFrame + mSampleOffsets[i]);
		i++;

		if (mDeviceNet->GetBitState() == mSettings->Dominant())
		{
			//the bit is DOMINANT
			mDominantCount++;
			mRecessiveCount = 0;
			mRawBitResults.push_back(mSettings->Dominant());

			if (mDominantCount == 6)
			{
				//we have detected an error.

				mCanError = true;
				mErrorStartingSample = mStartOfFrame + mSampleOffsets[i - 5];
				mErrorEndingSample = mStartOfFrame + mSampleOffsets[i];

				//don't use any of these error bits in analysis.
				mRawBitResults.pop_back();
				mRawBitResults.pop_back();
				mRawBitResults.pop_back();
				mRawBitResults.pop_back();
				mRawBitResults.pop_back();
				mRawBitResults.pop_back();

				mNumRawBits = mRawBitResults.size();

				//the channel is currently high.  addvance it to the next start bit.
				//no, don't bother, we want to analyze this packet before we advance.

				break;
			}
		}
		else
		{
			//the bit is RECESSIVE
			mRecessiveCount++;
			mDominantCount = 0;
			mRawBitResults.push_back(mSettings->Recessive());

			if (mRecessiveCount == 7)
			{
				//we're done.
				break;
			}

		}
	}

	mNumRawBits = mRawBitResults.size();
}



void DeviceNetAnalyzer::AnalizeRawFrame()
{
	BitState bit;
	U64 last_sample;

	UnstuffRawFrameBit(bit, last_sample, true);  //grab the start bit, and reset everything.
	mArbitrationField.clear();
	mControlField.clear();
	mDataField.clear();
	mCrcFieldWithoutDelimiter.clear();
	mAckField.clear();

	bool done;

	mIdentifier = 0;
	for (U32 i = 0; i < 11; i++)
	{
		mIdentifier <<= 1;
		BitState bit;
		done = UnstuffRawFrameBit(bit, last_sample);
		if (done == true)
			return;
		mArbitrationField.push_back(bit);

		if (bit == mSettings->Recessive())
			mIdentifier |= 1;
	}

	//ok, the next three bits will let us know if this is 11-bit or 29-bit can.  If it's 11-bit, then it'll also tell us if this is a remote frame request or not.

	BitState bit0;
	done = UnstuffRawFrameBit(bit0, last_sample);
	if (done == true)
		return;

	BitState bit1;
	done = UnstuffRawFrameBit(bit1, last_sample);
	if (done == true)
		return;

	//ok, if bit1 is dominant, then this is 11-bit. 

	Frame frame;

	if (bit1 == mSettings->Dominant())
	{
		//11-bit CAN

		BitState bit2;  //since this is 11-bit CAN, we know that bit2 is the r0 bit, which we are going to throw away.
		done = UnstuffRawFrameBit(bit2, last_sample);
		if (done == true)
			return;

		mStandardCan = true;

		frame.mStartingSampleInclusive = mStartOfFrame + mSampleOffsets[1];
		frame.mEndingSampleInclusive = last_sample;
		frame.mType = IdentifierField;

		if (bit0 == mSettings->Recessive()) //since this is 11-bit CAN, we know that bit0 is the RTR bit
		{
			mRemoteFrame = true;
			frame.mFlags = REMOTE_FRAME;
		}
		else
		{
			mRemoteFrame = false;
			frame.mFlags = 0;
		}

		frame.mData1 = mIdentifier;
		mResults->AddFrame(frame);
	}
	else
	{
		//29-bit CAN

		mStandardCan = false;

		//get the next 18 address bits.
		for (U32 i = 0; i < 18; i++)
		{
			mIdentifier <<= 1;

			BitState bit;
			done = UnstuffRawFrameBit(bit, last_sample);
			if (done == true)
				return;
			mArbitrationField.push_back(bit);

			if (bit == mSettings->Recessive())
				mIdentifier |= 1;
		}

		//get the RTR bit
		BitState rtr;
		done = UnstuffRawFrameBit(rtr, last_sample);
		if (done == true)
			return;

		//get the r0 and r1 bits (we won't use them)
		BitState r0;
		done = UnstuffRawFrameBit(r0, last_sample);
		if (done == true)
			return;

		BitState r1;
		done = UnstuffRawFrameBit(r1, last_sample);
		if (done == true)
			return;

		Frame frame;
		frame.mStartingSampleInclusive = mStartOfFrame + mSampleOffsets[1];
		frame.mEndingSampleInclusive = last_sample;
		frame.mType = IdentifierFieldEx;

		if (rtr == mSettings->Recessive())
		{
			mRemoteFrame = true;
			frame.mFlags = REMOTE_FRAME;
		}
		else
		{
			mRemoteFrame = false;
			frame.mFlags = 0;
		}

		frame.mData1 = mIdentifier;
		mResults->AddFrame(frame);
	}


	U32 mask = 0x8;
	mNumDataBytes = 0;
	U64 first_sample = 0;
	for (U32 i = 0; i < 4; i++)
	{
		BitState bit;
		if (i == 0)
			done = UnstuffRawFrameBit(bit, first_sample);
		else
			done = UnstuffRawFrameBit(bit, last_sample);

		if (done == true)
			return;

		mControlField.push_back(bit);

		if (bit == mSettings->Recessive())
			mNumDataBytes |= mask;

		mask >>= 1;
	}

	frame.mStartingSampleInclusive = first_sample;
	frame.mEndingSampleInclusive = last_sample;
	frame.mType = ControlField;
	frame.mData1 = mNumDataBytes;
	mResults->AddFrame(frame);

	U32 num_bytes = mNumDataBytes;
	if (num_bytes > 8)
		num_bytes = 8;

	if (mRemoteFrame == true)
		num_bytes = 0; //ignore the num_bytes if this is a remote frame.

	for (U32 i = 0; i < num_bytes; i++)
	{
		U32 data = 0;
		U32 mask = 0x80;
		for (U32 j = 0; j < 8; j++)
		{
			BitState bit;

			if (j == 0)
				done = UnstuffRawFrameBit(bit, first_sample);
			else
				done = UnstuffRawFrameBit(bit, last_sample);

			if (done == true)
				return;

			if (bit == mSettings->Recessive())
				data |= mask;

			mask >>= 1;

			mDataField.push_back(bit);
		}

		frame.mStartingSampleInclusive = first_sample;
		frame.mEndingSampleInclusive = last_sample;
		frame.mType = DataField;
		frame.mData1 = data;
		mResults->AddFrame(frame);
	}

	mCrcValue = 0;
	for (U32 i = 0; i < 15; i++)
	{
		mCrcValue <<= 1;
		BitState bit;

		if (i == 0)
			done = UnstuffRawFrameBit(bit, first_sample);
		else
			done = UnstuffRawFrameBit(bit, last_sample);

		if (done == true)
			return;

		mCrcFieldWithoutDelimiter.push_back(bit);

		if (bit == mSettings->Recessive())
			mCrcValue |= 1;
	}

	frame.mStartingSampleInclusive = first_sample;
	frame.mEndingSampleInclusive = last_sample;
	frame.mType = CrcField;
	frame.mData1 = mCrcValue;
	mResults->AddFrame(frame);

	done = UnstuffRawFrameBit(mCrcDelimiter, first_sample);

	if (done == true)
		return;

	BitState ack;
	done = GetFixedFormFrameBit(ack, first_sample);

	mAckField.push_back(ack);
	if (ack == mSettings->Dominant())
		mAck = true;
	else
		mAck = false;

	done = GetFixedFormFrameBit(ack, last_sample);

	if (done == true)
		return;

	mAckField.push_back(ack);

	frame.mStartingSampleInclusive = first_sample;
	frame.mEndingSampleInclusive = last_sample;
	frame.mType = AckField;
	frame.mData1 = mAck;
	mResults->AddFrame(frame);
	mResults->CommitPacketAndStartNewPacket();
}

bool DeviceNetAnalyzer::GetFixedFormFrameBit(BitState& result, U64& sample)
{
	if (mNumRawBits == mRawFrameIndex)
		return true;

	result = mRawBitResults[mRawFrameIndex];
	sample = mStartOfFrame + mSampleOffsets[mRawFrameIndex];
	mCanMarkers.push_back(CanMarker(sample, Standard));
	mRawFrameIndex++;

	return false;
}

bool DeviceNetAnalyzer::UnstuffRawFrameBit(BitState& result, U64& sample, bool reset)
{
	if (reset == true)
	{
		mRecessiveCount = 0;
		mDominantCount = 0;
		mRawFrameIndex = 0;
		mCanMarkers.clear();
	}

	if (mRawFrameIndex == mNumRawBits)
		return true;

	if (mRecessiveCount == 5)
	{
		mRecessiveCount = 0;
		mDominantCount = 1; //this bit is DOMINANT, and counts twards the next bit stuff
		mCanMarkers.push_back(CanMarker(mStartOfFrame + mSampleOffsets[mRawFrameIndex], BitStuff));
		mRawFrameIndex++;
	}

	if (mDominantCount == 5)
	{
		mDominantCount = 0;
		mRecessiveCount = 1; //this bit is RECESSIVE, and counts twards the next bit stuff
		mCanMarkers.push_back(CanMarker(mStartOfFrame + mSampleOffsets[mRawFrameIndex], BitStuff));
		mRawFrameIndex++;
	}

	if (mRawFrameIndex == mNumRawBits)
		return true;

	result = mRawBitResults[mRawFrameIndex];

	if (result == mSettings->Recessive())
	{
		mRecessiveCount++;
		mDominantCount = 0;
	}
	else
	{
		mDominantCount++;
		mRecessiveCount = 0;
	}

	sample = mStartOfFrame + mSampleOffsets[mRawFrameIndex];
	mCanMarkers.push_back(CanMarker(sample, Standard));
	mRawFrameIndex++;

	return false;
}