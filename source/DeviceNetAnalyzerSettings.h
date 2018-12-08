#ifndef DEVICENET_ANALYZER_SETTINGS
#define DEVICENET_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

enum BitRate
{
	BitRate_500K = 500000,
	BitRate_250K = 250000,
	BitRate_125K = 125000
};

class DeviceNetAnalyzerSettings : public AnalyzerSettings
{
public:
	DeviceNetAnalyzerSettings();
	virtual ~DeviceNetAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mDeviceNetChannel;
	enum BitRate mBitRate;
	bool mInverted;

	BitState Recessive();
	BitState Dominant();

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mDeviceNetChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceNumberList >	mBitRateInterface;
	std::auto_ptr< AnalyzerSettingInterfaceBool > mDeviceNetChannelInvertedInterface;
};

#endif //DEVICENET_ANALYZER_SETTINGS
