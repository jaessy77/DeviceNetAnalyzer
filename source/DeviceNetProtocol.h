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

// Address-Ranges MESSAGE GROUP's
#define START_ADDR_MESSAGE_GROUP_1		0x00000000
#define END_ADDR_MESSAGE_GROUP_1		0x000003FF
#define START_ADDR_MESSAGE_GROUP_2		0x00000400
#define END_ADDR_MESSAGE_GROUP_2		0x000005FF
#define START_ADDR_MESSAGE_GROUP_3		0x00000600
#define END_ADDR_MESSAGE_GROUP_3		0x000007BF
#define START_ADDR_MESSAGE_GROUP_4		0x000007C0
#define END_ADDR_MESSAGE_GROUP_4		0x000007EF
#define START_ADDR_INVALID_CAN_IDS		0x000007F0
#define END_ADDR_INVALID_CAN_IDS		0x000007FF
#define START_ADDR_INVALID_IDENTIFIER	0x00000800

// Address-Ranges CLASS ID's
#define START_ADDR_CLASS_ID_OPEN_1		0x00000000	// "Open"
#define END_ADDR_CLASS_ID_OPEN_1		0x00000063
#define START_ADDR_CLASS_ID_VENDOR_1	0x00000064	// "Vendor Specific"
#define END_ADDR_CLASS_ID_VENDOR_1		0x000000C7
#define START_ADDR_CLASS_ID_RESERVED_1	0x000000C8	// "Reserved by DeviceNet for future use"
#define END_ADDR_CLASS_ID_RESERVED_1	0x000000FF
#define START_ADDR_CLASS_ID_OPEN_2		0x00000100	// "Open"
#define END_ADDR_CLASS_ID_OPEN_2		0x000002FF
#define START_ADDR_CLASS_ID_VENDOR_2	0x00000300	// "Vendor Specific"
#define END_ADDR_CLASS_ID_VENDOR_2		0x000004FF
#define START_ADDR_CLASS_ID_RESERVED_2	0x00000500	// "Reserved by DeviceNet for future use"
#define END_ADDR_CLASS_ID_RESERVED_2	0x0000FFFF

// Address-Ranges SERVICE CODE's
#define START_ADDR_SERVICE_CODE_OPEN		0x00000000	// "Open.  These are referred to as DeviceNet Common Services.  These are defined in Appendix G."
#define END_ADDR_SERVICE_CODE_OPEN			0x00000031
#define START_ADDR_SERVICE_CODE_VENDOR		0x00000032	// "Vendor Specific"
#define END_ADDR_SERVICE_CODE_VENDOR		0x0000004A
#define START_ADDR_SERVICE_CODE_OBJECT		0x0000004B	// "Object Class Specific"
#define END_ADDR_SERVICE_CODE_OBJECT		0x00000063
#define START_ADDR_SERVICE_CODE_RESERVED	0x00000064	// "Reserved by DeviceNet for future use"
#define END_ADDR_SERVICE_CODE_RESERVED		0x0000007F
#define START_ADDR_SERVICE_CODE_INVALID		0x00000080	// "Invalid/Not used"
#define END_ADDR_SERVICE_CODE_INVALID		0x000000FF

// Address-Ranges ATTRIBUTE ID's
#define START_ADDR_ATTRIBUTE_ID_OPEN		0x00000000	// "Open"
#define END_ADDR_ATTRIBUTE_ID_OPEN			0x00000063
#define START_ADDR_ATTRIBUTE_ID_VENDOR		0x00000064	// "Vendor Specific"
#define END_ADDR_ATTRIBUTE_ID_VENDOR		0x000000C7
#define START_ADDR_ATTRIBUTE_ID_RESERVED	0x000000C8	// "Reserved by DeviceNet for future use"
#define END_ADDR_ATTRIBUTE_ID_RESERVED		0x000000FF

// Address-Ranges MAC ID's
#define START_ADDR_MAC_ID	0x00000000	// "The MAC ID.  The value 63 (decimal) is to be utilized upon initialization of a device (e.g. powerup) if another value has not been assigned."
#define END_ADDR_MAC_ID		0x0000003F

// Masks to decode and shift the Member's in Message Group's
#define MASK_GROUP_1_MESSAGE_ID		0x000003C0	// Bit 9:6
#define SHIFT_GROUP_1_MESSAGE_ID	6

#define MASK_SOURCE_MAC_ID			0x0000003F	// Bit 5:0 (Same position in MG1 and MG3)
#define SHIFT_SOURCE_MAC_ID			0

#define MASK_MAC_ID					0x000001F8	// Bit 8:3
#define SHIFT_MAC_ID				3

#define MASK_GROUP_2_MESSAGE_ID		0x00000007	// Bit 2:0
#define SHIFT_GROUP_2_MESSAGE_ID	0

#define MASK_GROUP_3_MESSAGE_ID		0x000001C0	// Bit 8:6
#define SHIFT_GROUP_3_MESSAGE_ID	6

#define MASK_GROUP_4_MESSAGE_ID		0x0000003F	// Bit 5:0
#define SHIFT_GROUP_4_MESSAGE_ID	0

// Identity usage bits in Field Identifier bit
#define BITS_MESSAGE_GROUP_1			0x00000000 // Bit 10 is always '0'
#define BITS_MESSAGE_GROUP_2			0x00000400 // Bit 10:9 is always '10'
#define BITS_MESSAGE_GROUP_3			0x00000600 // Bit 10:9 is always '11'
#define BITS_MESSAGE_GROUP_4			0x000007C0 // Bit 10:6 is always '11111'
#define BITS_INVALID_CAN_IDENTIFIERS	0x000007F0 // Bit 10:4 is always '1111111' Bit 3:0 is 'XXXX' (Do not care)

// Restriction Values MIN_VAL or MAX_VAL
#define MAX_VAL_GROUP_4_MESSAGE_ID		0x0000002F
#define MIN_VAL_INTERFRAME_SPACE_BITS	0x00000003	// Minimum 3 Bits "Recessive = '1' before next CAN-Frame"

// Check for Special Values
#define CHECK_GROUP_2_MSG_ID_IS_CONNECTION_MANAGEMENT		0x00000006	// "Reserved for Predefined Master/Slave Connection Management"
#define CHECK_GROUP_2_MSG_ID_IS_CHECK_MESSAGE				0x00000007	// "Duplicate MAC ID Check Message"
#define CHECK_GROUP_3_MSG_ID_IS_EXPLICIT_RESPONSE			0x00000005	// "Unconnected Explicit Response Messages"
#define CHECK_GROUP_3_MSG_ID_IS_EXPLICIT_REQUEST			0x00000006	// "Unconnected Explicit Request Messages"
#define CHECK_GROUP_3_MSG_ID_IS_RESERVED					0x00000007	// "Group 3 Message ID value 7 is invalid and is not used."
#define CHECK_GROUP_4_MSG_ID_IS_COM_FAULTED_RESPONSE		0x0000002C	// "Communication Faulted Response Message"
#define CHECK_GROUP_4_MSG_ID_IS_COM_FAULTED_REQUEST			0x0000002D	// "Communication Faulted Request Message"
#define CHECK_GROUP_4_MSG_ID_IS_OFFLINE_OWNERSHIP_RESPONSE	0x0000002E	// "Offline Ownership Response Message"
#define CHECK_GROUP_4_MSG_ID_IS_OFFLINE_OWNERSHIP_REQUEST	0x0000002F	// "Offline Ownership Request Message"

// Useful defines
#define BIT_RTR	0x00000000	// always dominant '0' since  DeviceNet does not make use of the CAN Remote Frame
#define BIT_R1	0x00000000	// always dominant '0' since DeviceNet is based on CAN 2.0A where r1 is reserved.
#define BIT_R0	0x00000000	// always dominant '0' since DeviceNet is based on CAN 2.0A where r0 is reserved.

#define LENGTH_ARBITRATION_FIELD	12
#define LENGTH_DATA_LENGTH_CODE		4
#define LENGTH_DATA_BYTE			8
#define LENGTH_END_OF_FRAME			7

enum DeviceNetFrameType
{
	IdentifierField,	// 11 bit Identifier, 1 bit RTR (always dominant '0' since  DeviceNet does not make use of the CAN Remote Frame)
	IdentifierFieldEx,	// In DeviceNet this Shound never occour !	
	ControlField,		// 2 bit Reserved (r1,r0 always '00'), 4 bit Data lenght code
	DataField,
	CrcField,
	AckField,
	DeviceNetError
};

#define REMOTE_FRAME ( 1 << 0 )

enum IdentifierType
{
	MessageGroup1,
	MessageGroup2,
	MessageGroup3,
	MessageGroup4,
	InvalidCanIdentifiers
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
	U32 mMacID;				// Member in Message Group 2 ; "either Source or Destination MAC ID"
	U32 mGroup2MessageID;	// Member in Message Group 2
	U32 mGroup3MessageID;	// Member in Message Group 3
	U32 mSourceMacID_MG3;	// Member in Message Group 3
	U32 mGroup4MessageID;	// Member in Message Group 4

	// Other useful variables
	bool mIdentifierError;
	bool mRtrBitError;
	bool mR1R0BitsError;
	bool mDataLegthError;

	// Group 2 Message ID specials
	bool mReservedMacIdConnectionManagement;	// Group 2 Message ID is '110' ; "Reserved for Predefined Master/Slave Connection Management"
	bool mDuplicateMacIdCheckMessage;			// Group 2 Message ID is '111' ; "Duplicate MAC ID Check Message"

	// Group 3 Message ID specials
	// Important: Use of Group 3 Message ID values 5, 6 and 7 is reserved by DeviceNet.
	bool mExplicitResponseMessage;				// Group 3 Message ID is '101' ; "Unconnected Explicit Response Messages"
	bool mExplicitRequestMessage;				// Group 3 Message ID is '110' ; "Unconnected Explicit Request Messages"
	bool mReservedGroup3MsgID;					// Group 3 Message ID is '111' ; "Group 3 Message ID value 7 is invalid and is not used."

	// Group 4 Message ID specials
	bool mReservedGroup4MsgID;					// Group 4 Message ID from '000000' to '101011' ; "Reserved Group 4 Message"
	bool mCommunicationFaultedResponseMessage;	// Group 4 Message ID  is '101100' ; "Communication Faulted Response Message"
	bool mCommunicationFaultedRequestMessage;	// Group 4 Message ID  is '101101' ; "Communication Faulted Request Message"
	bool mOfflineOwnershipResponseMessage;		// Group 4 Message ID  is '101110' ; "Offline Ownership Response Message"
	bool mOfflineOwnershipRequestMessage;		// Group 4 Message ID  is '101111' ; "Offline Ownership Request Message"
	
	U32 mDataLength;

	// Field variables
	U32 mArbitrationFieldIdentifierBits;	// 11 bits long
	U32 mArbitrationFieldRtrBit;			// 1 bit long, always '0'
	U32 mArbitrationField;					// 12 bits ( mArbitrationFieldIdentifierBits + mArbitrationFieldRtrBit)
	
	U32 mControlFieldReservedBits;			// 2 bit long, always '00'
	U32 mControlFieldDataLengthCodeBits;	// 4 bit long
	U32 mControlField;						// 6 bits long (mControlFieldReservedBits + mControlFieldDataLengthCodeBits)
	
	// Simulation dependend functions
	void ComposeGroup1MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1);
	void ComposeGroup2MessageIdentifier(U32 mGroup2MessageID, U32 mMacID);
	void ComposeGroup3MessageIdentifier(U32 mGroup3MessageID, U32 mSourceMacID_MG3);
	void ComposeGroup4MessageIdentifier(U32 mGroup4MessageID);
	void ComposeArbitrationField();

	void ComposeControlField(U32 mDataLengthCode);

	// Analyzing dependend functions
	void DecomposeArbitrationField(U32 mArbitrationField);

	void DecomposeControlField(U32 mControlField);

protected:
	
};

#endif //DEVICENET_PROTOCOL