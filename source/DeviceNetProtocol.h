#ifndef DEVICENET_PROTOCOL
#define DEVICENET_PROTOCOL

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

/*	DeviceNet specific

	Message Group 1
	11 bits, HEX-range 000 .. 3ff
	1 0 0 0 0 0 0 0 0 0 0
	0 9 8 7 6 5 4 3 2 1 0
	---------------------
	0 X X X X Y Y Y Y Y Y
	| |.....| |.........|
	| |.....| |_________|....===>	Source MAC ID		Bit 5:0
	| |_____|................===>	Group 1 Message ID	Bit 9:6
	|........................===>	Message Group 1		Bit 10 always '0'

	Message Group 2
	11 bits, HEX-range 400 .. 5ff
	1 0 0 0 0 0 0 0 0 0 0
	0 9 8 7 6 5 4 3 2 1 0
	---------------------
	1 0 X X X X X X Y Y Y
	|.| |.........| |...|
	|.| |.........| |___|....===>	Group 2 Message ID	Bit 2:0
	|.| |_________|..........===>	MAC ID				Bit 8:3
	|_|......................===>	Message Group 2		Bit 10:9 always '10'

	Message Group 3
	11 bits, HEX-range 600 .. 7bf
	1 0 0 0 0 0 0 0 0 0 0
	0 9 8 7 6 5 4 3 2 1 0
	---------------------
	1 1 X X X Y Y Y Y Y Y
	|.| |...| |.........|
	|.| |...| |_________|....===>	Source MAC ID		Bit 5:0
	|.| |___|................===>	Group 3 Message ID	Bit 8:6
	|_|......................===>	Message Group 3		Bit 10:9 always '11'

	Message Group 4
	11 bits, HEX-range 7c0 .. 7ef
	1 0 0 0 0 0 0 0 0 0 0
	0 9 8 7 6 5 4 3 2 1 0
	---------------------
	1 1 1 1 1 X X X X X X
	|.......| |_________|....===>	Group 4 Message ID	Bit 5:0
	|_______|................===>	Message Group 4		Bit 10:6 always '11111'

	Invalid CAN Identifiers
	11 bits, HEX-range 7f0 .. 7ff
	1 0 0 0 0 0 0 0 0 0 0
	0 9 8 7 6 5 4 3 2 1 0
	---------------------
	1 1 1 1 1 1 1 X X X X
	|...........| |_____|....===>	Do not care ..		Bit 3:0
	|___________|............===>	Invalid CAN ID		Bit 10:4 always '1111111'
*/

// Address-Ranges
#define MESSAGE_GROUP_1_START		0x00000000
#define MESSAGE_GROUP_1_END			0x000003FF
#define MESSAGE_GROUP_2_START		0x00000400
#define MESSAGE_GROUP_2_END			0x000005FF
#define MESSAGE_GROUP_3_START		0x00000600
#define MESSAGE_GROUP_3_END			0x000007BF
#define MESSAGE_GROUP_4_START		0x000007C0
#define MESSAGE_GROUP_4_END			0x000007EF
#define INVALID_CAN_IDS_START		0x000007F0
#define INVALID_CAN_IDS_END			0x000007FF
#define INVALID_IDENTIFIER_START	0x00000800

// Masks to decode and shift the Member's in Message Group's
#define GROUP_1_MESSAGE_ID_MASK		0x000003C0	// Bit 9:6
#define GROUP_1_MESSAGE_ID_SHIFT	6

#define SOURCE_MAC_ID_MASK			0x0000003F	// Bit 5:0 (Same position in MG1 and MG3)
#define SOURCE_MAC_ID_SHIFT			0

#define MAC_ID_MASK					0x000001F8	// Bit 8:3
#define MAC_ID_SHIFT				3

#define GROUP_2_MESSAGE_ID_MASK		0x00000007	// Bit 2:0
#define GROUP_2_MESSAGE_ID_SHIFT	0

#define GROUP_3_MESSAGE_ID_MASK		0x000001C0	// Bit 8:6
#define GROUP_3_MESSAGE_ID_SHIFT	6

#define GROUP_4_MESSAGE_ID_MASK		0x0000003F	// Bit 5:0
#define GROUP_4_MESSAGE_ID_SHIFT	0

enum DeviceNetFrameType
{
	IdentifierField,
	ControlField,
	DataField,
	CrcField,
	AckField,
	DeviceNetError
};

class DeviceNetProtocol
{
public:
	DeviceNetProtocol();
	virtual ~DeviceNetProtocol();

	// Identity usage
	bool mMessageGroup1;
	bool mMessageGroup2;
	bool mMessageGroup3;
	bool mMessageGroup4;
	bool mInvalidCanIdentifiers;

	// ID-Types included in identifier bits
	U32 mGroup1MessageID;	// Member in Message Group 1	
	U32 mSourceMacID_MG1;	// Member in Message Group 1
	U32 mMacID;				// Member in Message Group 2 ; either Source or Destination MAC ID
	U32 mGroup2MessageID;	// Member in Message Group 2
	U32 mGroup3MessageID;	// Member in Message Group 3
	U32 mSourceMacID_MG2;	// Member in Message Group 3
	U32 mGroup4MessageID;	// Member in Message Group 4

	// Other useful variables
	bool mIdentifierError;

	// Field variables
	U32 mIdentifierBits;	// 11 bits long

	U32 ComposeGroup1MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1);
	U32 ComposeGroup2MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1);
	U32 ComposeGroup3MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1);
	U32 ComposeGroup4MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1);

protected:
	
};

#endif //DEVICENET_PROTOCOL