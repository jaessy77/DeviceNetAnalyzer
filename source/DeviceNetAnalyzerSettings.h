#ifndef DEVICENET_ANALYZER_SETTINGS
#define DEVICENET_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DeviceNetAnalyzerSettings : public AnalyzerSettings
{
public:
	DeviceNetAnalyzerSettings();
	virtual ~DeviceNetAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	U32 mBitRate;

protected:
	std::auto_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::auto_ptr< AnalyzerSettingInterfaceInteger >	mBitRateInterface;
};

#endif //DEVICENET_ANALYZER_SETTINGS
