#ifndef DEVICENET_SIMULATION_DATA_GENERATOR
#define DEVICENET_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <AnalyzerHelpers.h>

class DeviceNetAnalyzerSettings;

class DeviceNetSimulationDataGenerator
{
public:
	DeviceNetSimulationDataGenerator();
	~DeviceNetSimulationDataGenerator();

	void Initialize( U32 simulation_sample_rate, DeviceNetAnalyzerSettings* settings );
	U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel );

protected:
	DeviceNetAnalyzerSettings* mSettings;
	U32 mSimulationSampleRateHz;

protected: // fuctions
	void CreateDataFrame(enum IdentifierType idType, U8 GroupMessageID, U8 MacID, std::vector<U8>& data, bool get_ack_in_response);
	void AddCrc();
	U16 ComputeCrc(std::vector<BitState>& bits, U32 num_bits);
	void WriteFrame(bool error = false);

protected: //vars
	ClockGenerator mClockGenerator;
	SimulationChannelDescriptor mDeviceNetSimulationData;  //if we had more than one channel to simulate, they would need to be in an array

	U8 mValue;

	std::vector<BitState> mFakeStartOfFrameField;
	std::vector<BitState> mFakeArbitrationField;
	std::vector<BitState> mFakeControlField;
	std::vector<BitState> mFakeDataField;
	std::vector<BitState> mFakeCrcFieldWithoutDelimiter;
	std::vector<BitState> mFakeAckField;
	std::vector<BitState> mFakeEndOfFrame;
	std::vector<BitState> mFakeStuffedBits;
	std::vector<BitState> mFakeFixedFormBits;

	



};
#endif //DEVICENET_SIMULATION_DATA_GENERATOR