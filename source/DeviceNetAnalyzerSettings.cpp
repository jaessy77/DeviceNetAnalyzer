#include "DeviceNetAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


DeviceNetAnalyzerSettings::DeviceNetAnalyzerSettings()
:	mDeviceNetChannel( UNDEFINED_CHANNEL ),
	mBitRate( BitRate_500K ),
	mInverted(false)
{
	mDeviceNetChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mDeviceNetChannelInterface->SetTitleAndTooltip( "DeviceNet", "Standard DeviceNet (based on CAN2.0A)" );
	mDeviceNetChannelInterface->SetChannel( mDeviceNetChannel );

	mBitRateInterface.reset( new AnalyzerSettingInterfaceNumberList() );
	mBitRateInterface->SetTitleAndTooltip( "Bit Rate (Bits/S)",  "Specify the bit rate in bits per second." );
	mBitRateInterface->AddNumber(BitRate_500K, "500000 Bits/s" , "High-Speed DeviceNet" );
	mBitRateInterface->AddNumber(BitRate_250K, "250000 Bits/s" , "Middle-Speed DeviceNet" );
	mBitRateInterface->AddNumber(BitRate_125K, "125000 Bits/s" , "Low-Speed DeviceNet" );
	mBitRateInterface->SetNumber(mBitRate);

	mDeviceNetChannelInvertedInterface.reset(new AnalyzerSettingInterfaceBool());
	mDeviceNetChannelInvertedInterface->SetTitleAndTooltip("Inverted (CAN High)", "Use this option when recording CAN High directly");
	mDeviceNetChannelInvertedInterface->SetValue(mInverted);

	AddInterface( mDeviceNetChannelInterface.get() );
	AddInterface( mBitRateInterface.get() );
	AddInterface( mDeviceNetChannelInvertedInterface.get());

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mDeviceNetChannel, "DeviceNet", false );
}

DeviceNetAnalyzerSettings::~DeviceNetAnalyzerSettings()
{
}

bool DeviceNetAnalyzerSettings::SetSettingsFromInterfaces()
{
	mDeviceNetChannel = mDeviceNetChannelInterface->GetChannel();
	mBitRate = BitRate( U32 (mBitRateInterface->GetNumber() ) );
	mInverted = mDeviceNetChannelInvertedInterface->GetValue();

	ClearChannels();
	AddChannel( mDeviceNetChannel, "DeviceNet", true );

	return true;
}

void DeviceNetAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mDeviceNetChannelInterface->SetChannel( mDeviceNetChannel );
	mBitRateInterface->SetNumber( mBitRate );
	mDeviceNetChannelInvertedInterface->SetValue(mInverted);
}

void DeviceNetAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mDeviceNetChannel;
	text_archive >> * (U32*) &mBitRate;

	ClearChannels();
	AddChannel( mDeviceNetChannel, "DeviceNet", true );

	UpdateInterfacesFromSettings();
}

const char* DeviceNetAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mDeviceNetChannel;
	text_archive << mBitRate;

	return SetReturnString( text_archive.GetString() );
}

BitState DeviceNetAnalyzerSettings::Recessive()
{
	if (mInverted)
		return BIT_LOW;
	return BIT_HIGH;
}
BitState DeviceNetAnalyzerSettings::Dominant()
{
	if (mInverted)
		return BIT_HIGH;
	return BIT_LOW;
}
