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
	mSourceMacID_MG3 = 0;	// Member in Message Group 3
	mGroup4MessageID = 0;	// Member in Message Group 4

	// Other useful variables
	mIdentifierError = false;
	mRtrBitError = false;
	mR1R0BitsError = false;
	mDataLegthError = false;

	// Group 2 Message ID specials
	mReservedMacIdConnectionManagement = false;
	mDuplicateMacIdCheckMessage = false;

	// Group 3 Message ID specials
	mExplicitResponseMessage = false;
	mExplicitRequestMessage = false;
	mReservedGroup3MsgID = false;

	// Group 4 Message ID specials
	mReservedGroup4MsgID = false;
	mCommunicationFaultedResponseMessage = false;
	mCommunicationFaultedRequestMessage = false;
	mOfflineOwnershipResponseMessage = false;
	mOfflineOwnershipRequestMessage = false;

	// Field variables
	mArbitrationFieldIdentifierBits = 0;
	mArbitrationFieldRtrBit = BIT_RTR;
	mArbitrationField = 0;

	mControlFieldReservedBits = BIT_R1 | BIT_R0;
	mControlFieldDataLengthCodeBits = 0;
	mControlField = 0;


}

DeviceNetProtocol::~DeviceNetProtocol()
{
}

void DeviceNetProtocol::ComposeGroup1MessageIdentifier(U32 mGroup1MessageID, U32 mSourceMacID_MG1)
{
	mArbitrationFieldIdentifierBits = BITS_MESSAGE_GROUP_1 | (mGroup1MessageID << SHIFT_GROUP_1_MESSAGE_ID) | (mSourceMacID_MG1 << SHIFT_SOURCE_MAC_ID);
}

void DeviceNetProtocol::ComposeGroup2MessageIdentifier(U32 mGroup2MessageID, U32 mMacID)
{
	mArbitrationFieldIdentifierBits = BITS_MESSAGE_GROUP_2 | (mGroup2MessageID << SHIFT_GROUP_2_MESSAGE_ID) | (mMacID << SHIFT_MAC_ID);
}

void DeviceNetProtocol::ComposeGroup3MessageIdentifier(U32 mGroup3MessageID, U32 mSourceMacID_MG3)
{
	mArbitrationFieldIdentifierBits = BITS_MESSAGE_GROUP_3 | (mGroup3MessageID << SHIFT_GROUP_3_MESSAGE_ID) | (mSourceMacID_MG3 << SHIFT_SOURCE_MAC_ID);
}

void DeviceNetProtocol::ComposeGroup4MessageIdentifier(U32 mGroup4MessageID)
{
	if (mGroup4MessageID > MAX_VAL_GROUP_4_MESSAGE_ID)
	{
		mArbitrationFieldIdentifierBits = 0xFFFFFFFF;
	}
		
	else
	{
		mArbitrationFieldIdentifierBits = BITS_MESSAGE_GROUP_4 | (mGroup4MessageID << SHIFT_GROUP_4_MESSAGE_ID);
	}	
}

void DeviceNetProtocol::ComposeArbitrationField(void)
{
	mArbitrationField = (mArbitrationFieldIdentifierBits << 1) | (mArbitrationFieldRtrBit << 0);

}

void DeviceNetProtocol::ComposeControlField(U32 mDataLengthCode)
{
	mControlFieldDataLengthCodeBits = mDataLengthCode;
	mControlField = (mControlFieldReservedBits << 4) | (mControlFieldDataLengthCodeBits << 0);
}

void DeviceNetProtocol::DecomposeArbitrationField(U32 mArbitrationField)
{
	mArbitrationFieldIdentifierBits = ((mArbitrationField & 0x00000FFE ) >> 1);
	mArbitrationFieldRtrBit = ((mArbitrationField & 0x00000001) >> 0);

	// Decode the identifier-bits to figure out the Message Group
	if ((mArbitrationFieldIdentifierBits >= START_ADDR_MESSAGE_GROUP_1) && (mArbitrationFieldIdentifierBits < START_ADDR_MESSAGE_GROUP_2))
	{
		mMessageGroup1 = true;
		mGroup1MessageID = ((mArbitrationFieldIdentifierBits & MASK_GROUP_1_MESSAGE_ID) >> SHIFT_GROUP_1_MESSAGE_ID);
		mSourceMacID_MG1 = ((mArbitrationFieldIdentifierBits & MASK_SOURCE_MAC_ID) >> SHIFT_SOURCE_MAC_ID);
	}
	else if ((mArbitrationFieldIdentifierBits >= START_ADDR_MESSAGE_GROUP_2) && (mArbitrationFieldIdentifierBits < START_ADDR_MESSAGE_GROUP_3))
	{
		mMessageGroup2 = true;
		mMacID = ((mArbitrationFieldIdentifierBits & MASK_MAC_ID) >> SHIFT_MAC_ID);
		mGroup2MessageID = ((mArbitrationFieldIdentifierBits & MASK_GROUP_2_MESSAGE_ID) >> SHIFT_GROUP_2_MESSAGE_ID);
		if (mGroup2MessageID == CHECK_GROUP_2_MSG_ID_IS_CONNECTION_MANAGEMENT)
		{
			mReservedMacIdConnectionManagement = true;
		}
		if (mGroup2MessageID == CHECK_GROUP_2_MSG_ID_IS_CHECK_MESSAGE)
		{
			mDuplicateMacIdCheckMessage = true;
		}
	}
	else if ((mArbitrationFieldIdentifierBits >= START_ADDR_MESSAGE_GROUP_3) && (mArbitrationFieldIdentifierBits < START_ADDR_MESSAGE_GROUP_4))
	{
		mMessageGroup3 = true;

		mGroup3MessageID = ((mArbitrationFieldIdentifierBits & MASK_GROUP_3_MESSAGE_ID) >> SHIFT_GROUP_3_MESSAGE_ID);

		if (mGroup3MessageID == CHECK_GROUP_3_MSG_ID_IS_EXPLICIT_RESPONSE)
		{
			mExplicitResponseMessage = true;
		}
		if (mGroup3MessageID == CHECK_GROUP_3_MSG_ID_IS_EXPLICIT_REQUEST)
		{
			mExplicitRequestMessage = true;
		}
		if (mGroup3MessageID == CHECK_GROUP_3_MSG_ID_IS_RESERVED)
		{
			mReservedGroup3MsgID = true;
		}

		mSourceMacID_MG3 = ((mArbitrationFieldIdentifierBits & MASK_SOURCE_MAC_ID) >> SHIFT_SOURCE_MAC_ID);
	}
	else if ((mArbitrationFieldIdentifierBits >= START_ADDR_MESSAGE_GROUP_4) && (mArbitrationFieldIdentifierBits < START_ADDR_INVALID_CAN_IDS))
	{
		mMessageGroup4 = true;

		mGroup4MessageID = ((mArbitrationFieldIdentifierBits & MASK_GROUP_4_MESSAGE_ID) >> SHIFT_GROUP_4_MESSAGE_ID);

		if (mGroup4MessageID == CHECK_GROUP_4_MSG_ID_IS_COM_FAULTED_RESPONSE)
		{
			mCommunicationFaultedResponseMessage = true;
		}
		else if (mGroup4MessageID == CHECK_GROUP_4_MSG_ID_IS_COM_FAULTED_REQUEST)
		{
			mCommunicationFaultedRequestMessage = true;
		}
		else if (mGroup4MessageID == CHECK_GROUP_4_MSG_ID_IS_OFFLINE_OWNERSHIP_RESPONSE)
		{
			mOfflineOwnershipResponseMessage = true;
		}
		else if (mGroup4MessageID == CHECK_GROUP_4_MSG_ID_IS_OFFLINE_OWNERSHIP_REQUEST)
		{
			mOfflineOwnershipRequestMessage = true;
		}
		else
		{
			mReservedGroup4MsgID = true;
		}
	}
	else if ((mArbitrationFieldIdentifierBits >= START_ADDR_INVALID_CAN_IDS) && (mArbitrationFieldIdentifierBits < START_ADDR_INVALID_IDENTIFIER))
	{
		mInvalidCanIdentifiers = true;
	}
	else
	{
		mIdentifierError = true;
	}

	if (mArbitrationFieldRtrBit != BIT_RTR)
	{
		mRtrBitError = true;
	}
}

void DeviceNetProtocol::DecomposeControlField(U32 mControlField)
{
	mControlFieldReservedBits = ((mControlField & 0x00000030) >> 4);
	mControlFieldDataLengthCodeBits = ((mControlField & 0x0000000F) >> 0);

	if (mControlFieldReservedBits != (BIT_R1 | BIT_R0))
	{
		mR1R0BitsError = true;
	}

	if (mControlFieldDataLengthCodeBits == 0)
	{
		mDataLegthError = true;
		mDataLength = 0;
	}
	else
	{
		mDataLength = mControlFieldDataLengthCodeBits;
	}

}