#include "DeviceNetProtocol.h"

DeviceNetProtocol::DeviceNetProtocol()
{
	// Identity usage
	mMessageGroup1 = false;
	mMessageGroup2 = false;
	mMessageGroup3 = false;
	mMessageGroup4 = false;
	mInvalidCanIdentifiers = false;

	// ID-Types included in identifier bits
	mGroup1MessageID = 0;	// Member in Message Group 1	
	mSourceMacID_MG1 = 0;	// Member in Message Group 1
	mMacID = 0;				// Member in Message Group 2 ; either Source or Destination MAC ID
	mGroup2MessageID = 0;	// Member in Message Group 2
	mGroup3MessageID = 0;	// Member in Message Group 3
	mSourceMacID_MG2 = 0;	// Member in Message Group 3
	mGroup4MessageID = 0;	// Member in Message Group 4

	// Other useful variables
	mIdentifierError = false;
}

DeviceNetProtocol::~DeviceNetProtocol()
{
}


U32 DeviceNetProtocol::ComposeGroup1MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1)
{
	return U32();
}

U32 DeviceNetProtocol::ComposeGroup2MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1)
{
	return U32();
}

U32 DeviceNetProtocol::ComposeGroup3MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1)
{
	return U32();
}

U32 DeviceNetProtocol::ComposeGroup4MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1)
{
	return U32();
}