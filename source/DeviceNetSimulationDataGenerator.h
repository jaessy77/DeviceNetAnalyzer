#ifndef DEVICENET_SIMULATION_DATA_GENERATOR
#define DEVICENET_SIMULATION_DATA_GENERATOR

#include <SimulationChannelDescriptor.h>
#include <string>
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

protected:
	void CreateSerialByte();
	std::string mSerialText;
	U32 mStringIndex;

	SimulationChannelDescriptor mSerialSimulationData;

};
#endif //DEVICENET_SIMULATION_DATA_GENERATOR