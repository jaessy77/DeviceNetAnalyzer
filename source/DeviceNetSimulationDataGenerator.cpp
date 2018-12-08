#include "DeviceNetSimulationDataGenerator.h"
#include "DeviceNetAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

#include "DeviceNetProtocol.h"

DeviceNetSimulationDataGenerator::DeviceNetSimulationDataGenerator()
{
}

DeviceNetSimulationDataGenerator::~DeviceNetSimulationDataGenerator()
{
}

void DeviceNetSimulationDataGenerator::Initialize( U32 simulation_sample_rate, DeviceNetAnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mClockGenerator.Init(mSettings->mBitRate, simulation_sample_rate);

	mDeviceNetSimulationData.SetChannel( mSettings->mDeviceNetChannel );
	mDeviceNetSimulationData.SetSampleRate( simulation_sample_rate );
	mDeviceNetSimulationData.SetInitialBitState( BIT_HIGH );

	mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(10.0));  //insert 10 bit-periods of idle

	mValue = 0;
}

U32 DeviceNetSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	std::vector<U8> data;
	std::vector<U8> empty_data;

	while( mDeviceNetSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
		data.clear();
		data.push_back(mValue + 0);
		data.push_back(mValue + 1);
		data.push_back(mValue + 2);
		data.push_back(mValue + 3);

		data.push_back(mValue + 4);
		data.push_back(mValue + 5);
		data.push_back(mValue + 6);
		data.push_back(mValue + 7);

		mValue++;

		CreateDataFrame(MessageGroup1, 0xC, 0x2F, data, true);
		WriteFrame();
		CreateDataFrame(MessageGroup1, 0xC, 0x2F, data, false);
		WriteFrame();
	}

	*simulation_channel = &mDeviceNetSimulationData;
	return 1;
}

void DeviceNetSimulationDataGenerator::CreateDataFrame(enum IdentifierType idType, U8 GroupMessageID, U8 MacID, std::vector<U8>& data, bool get_ack_in_response)
{
	U32 samples_per_bit = mSimulationSampleRateHz / mSettings->mBitRate;

	/*!
	 * Data Frame
	 * 
	 * A DATA FRAME is composed of seven different bit fields:
	 * - START OF FRAME
	 * - ARBITRATION FIELD
	 * - CONTROL FIELD
	 * - DATA FIELD
	 * - CRC FIELD
	 * - ACK FIELD
	 * - END OF FRAME.
	 * 
	 * The DATA FIELD can be of length zero. @Todo: Check this with DeviceNet
	 * 
	 */ 
	mFakeStuffedBits.clear();
	mFakeFixedFormBits.clear();
	mFakeStartOfFrameField.clear();
	mFakeArbitrationField.clear();
	mFakeControlField.clear();
	mFakeDataField.clear();
	mFakeCrcFieldWithoutDelimiter.clear();
	mFakeAckField.clear();
	mFakeEndOfFrame.clear();

	/*!
	 * START OF FRAME (Standard Format)
	 * 
	 * The START OF FRAME (SOF) marks the beginning of DATA FRAMES.
	 * REMOTE FRAMEs exists in CAN, but not used in DeviceNet.
	 * 
	 * It consists of a single dominant bit.
	 * 
	 * A station is only allowed to start transmission when the bus is idle (see INTERFRAME Spacing).
	 * All stations have to synchronize to the leading edge caused by START OF FRAME
	 * (see HARD SYNCHRONIZATION) of the station starting transmission first.
	 */

	mFakeStartOfFrameField.push_back(mSettings->Dominant());

	/*!
	 * ARBITRATION FIELD
	 * 
	 * The format of the ARBITRATION FIELD is normally different for Standard Format and Extended Format Frames.
	 * 
	 * Since DeviceNet does not make use of Extended Format Frames we do not care about that (yet).
	 * 
	 * In Standard Format the ARBITRATION FIELD consists of the 11 bit IDENTIFIER and the RTR-BIT.
	 * 
	 * - IDENTIFIER (Standard Format)
	 * 
	 * The IDENTIFIERs length is 11 bits in CAN. In a Standard Frame the IDENTIFIER is followed by the RTR bit.
	 * 
	 * In DeviceNet the 11 bits IDENTIFIER is splitted in 4 Message Groups and Invalid CAN Identifiers.
	 * 
	 * - RTR BIT (Standard Format)
	 * 
	 * Remote Transmission Request Bit is in DATA FRAMEs the RTR BIT and has to be dominant
	 * 
	 * Within a REMOTE FRAME the RTR BIT has to be recessive, but DeviceNet does not make use uf REMOTE FRAMES,
	 * therefore we can forget this for now.
	 */

	U32 mask = 1 << (LENGTH_ARBITRATION_FIELD - 1);
	U32 identifier;

	if (idType == MessageGroup1)
	{
		identifier = BITS_MESSAGE_GROUP_1 | (GroupMessageID << SHIFT_GROUP_1_MESSAGE_ID) | (MacID << SHIFT_SOURCE_MAC_ID);
	}
	else if (idType == MessageGroup2)
	{
		identifier = BITS_MESSAGE_GROUP_2 | (GroupMessageID << SHIFT_GROUP_2_MESSAGE_ID) | (MacID << SHIFT_MAC_ID);
	}
	else if (idType == MessageGroup3)
	{
		identifier = BITS_MESSAGE_GROUP_3 | (GroupMessageID << SHIFT_GROUP_3_MESSAGE_ID) | (MacID << SHIFT_SOURCE_MAC_ID);
	}
	else if (idType == MessageGroup4)
	{
		if (GroupMessageID > MAX_VAL_GROUP_4_MESSAGE_ID)
		{
			identifier = 0xFFFFFFF0;
			AnalyzerHelpers::Assert("Group 4 Message ID must be in range of 0x00 .. 0x2F");
		}

		else
		{
			identifier = BITS_MESSAGE_GROUP_4 | (GroupMessageID << SHIFT_GROUP_4_MESSAGE_ID);
		}
	}
	else if (idType == InvalidCanIdentifiers)
	{
		identifier = 0xFFFFFFF0;
	}
	else
	{
		identifier = 0xFFFFFFFF;
	}

	for (U32 i = 0; i < LENGTH_ARBITRATION_FIELD; i++)
	{
		if ((mask & identifier) == 0)
			mFakeArbitrationField.push_back(mSettings->Dominant());
		else
			mFakeArbitrationField.push_back(mSettings->Recessive());

		mask >>= 1;
	}

	/*!
	 * The next bit is RTR
	 * 
	 * - RTR BIT (Standard Format)
	 * Remote Transmission Request Bit
	 * 
	 * In DATA FRAMEs the RTR BIT has to be dominant.
	 * Within a REMOTE FRAME the RTR BIT has to be recessive.
	 * 
	 * Since in DeviceNet a REMOTE FRAME does not exist, we set it permanently dominant.
	 */
	  
	mFakeArbitrationField.push_back(mSettings->Dominant()); //RTR bit always '0'

	 /*!
	  * 
	  * Next is the control field with is built up
	  * r1 r0 DLC3 DLC2 DLC1 DLC0
	  * 
	  * CONTROL FIELD (Standard Format)
	  * 
	  * The CONTROL FIELD consists of six bits.
	  * The format of the CONTROL FIELD is different for Standard Format and Extended Format.
	  * Frames in Standard Format include the DATA LENGTH CODE, the IDE bit (r1), which is transmitted dominant
	  * and the reserved bit r0 which is always dent dominant.
	  */

	mFakeControlField.push_back(mSettings->Dominant()); //r1 bit (IDE)
	mFakeControlField.push_back(mSettings->Dominant()); //r0 bit

	/*!
	 * 
	 * DLC
	 * Data Length Code
	 * 
	 * Send 4 bits for the length of the attached data.
	 * 
	 */
	U32 data_size = data.size();
	if (data_size > 9)
		AnalyzerHelpers::Assert("DeviceNet can't sent more than 8 bytes");
	
	// @ TODO: Check if a empty dataframe is possible in DeviceNet
	//if (data_size == 0)
	//	AnalyzerHelpers::Assert("remote frames can't send data");

	mask = 1 << (LENGTH_DATA_LENGTH_CODE - 1 );
	for (U32 i = 0; i < LENGTH_DATA_LENGTH_CODE; i++)
	{
		if ((mask & data_size) == 0)
			mFakeControlField.push_back(mSettings->Dominant());
		else
			mFakeControlField.push_back(mSettings->Recessive());

		mask >>= 1;
	}

	/*!
	 * 
	 * Next is the DATA FIELD
	 * 
	 * DATA FIELD (Standard Format)
	 * 
	 * The DATA FIELD consists of the data to be transferred within a DATA FRAME.
	 * It can contain from 0 to 8 bytes, which each contain 8 bits which are transferred MSB first.
	 * 
	 * @ TODO: Check if 0 Bytes really exists in DeviceNet ..
	 * 
	 */

	for (U32 i = 0; i < data_size; i++)
	{
		U32 dat = data[i];
		U32 mask = 1 << (LENGTH_DATA_BYTE - 1);

		for (U32 j = 0; j < LENGTH_DATA_BYTE; j++)
		{
			if ((mask & dat) == 0)
				mFakeDataField.push_back(mSettings->Dominant());
			else
				mFakeDataField.push_back(mSettings->Recessive());

			mask >>= 1;
		}
	}

	/*!
	 * 
	 * CRC FIELD (Standard Format)
	 * 
	 * Contains the CRC SEQUENCE followed by a CRC DELIMITER.
	 */
	

	AddCrc();

	/*!
	 * 
	 * ACK FIELD (Standard Format)
	 * 
	 * The ACK FIELD is two bits long and contains the ACK SLOT and the ACK DELIMITER.
	 * 
	 * In the ACK FIELD the transmitting station sends two recessive bits.
	 * A RECEIVER which has received a valid message correctly, reports this to the
	 * TRANSMITTER by sending a dominant bit during the ACK SLOT (it sends ACK).
	 */

	/*!
	 * ACK SLOT
	 * 
	 * All stations having received the matching CRC SEQUENCE report this within
	 * the ACK SLOT by superscribing the recessive bit of the TRANSMITTER by a dominant bit.
	 */

	if (get_ack_in_response == true)
		mFakeAckField.push_back(mSettings->Dominant());
	else
		mFakeAckField.push_back(mSettings->Recessive());

	/*!
	 * ACK DELIMITER
	 * 
	 * The ACK DELIMITER is the second bit of the ACK FIELD and has to be a recessive bit.
	 * As a consequence, the ACK SLOT is surrounded by two recessive bits (CRC DELIMITER, ACK DELIMITER).
	 */

	mFakeAckField.push_back(mSettings->Recessive());

	/*!
	 * END OF FRAME (Standard Format)
	 * 
	 * Each DATA FRAME and REMOTE FRAME is delimited by a flag sequence consisting
	 * of seven recessive bits.
	 */

	for (U32 i = 0; i < LENGTH_END_OF_FRAME; i++)
		mFakeEndOfFrame.push_back(mSettings->Recessive());


	mFakeFixedFormBits.insert(mFakeFixedFormBits.end(), mFakeAckField.begin(), mFakeAckField.end());
	mFakeFixedFormBits.insert(mFakeFixedFormBits.end(), mFakeEndOfFrame.begin(), mFakeEndOfFrame.end());


	DeviceNetProtocol mDeviceNet;

	/*
	U8 byte = mSerialText[ mStringIndex ];
	mStringIndex++;
	if( mStringIndex == mSerialText.size() )
		mStringIndex = 0;
	*/

	

	//we're currenty high
	//let's move forward a little
	mDeviceNetSimulationData.Advance( samples_per_bit * 10 );

	mDeviceNetSimulationData.Transition();  //low-going edge for start bit
	mDeviceNetSimulationData.Advance( samples_per_bit );  //add start bit time

	//build the Arbitration Field

	mDeviceNet.ComposeGroup1MessageIdentifier(0xC, 0x38);
	mDeviceNet.ComposeArbitrationField();

	mask = 0x1 << (LENGTH_ARBITRATION_FIELD - 1);
	for( U32 i=0; i< LENGTH_ARBITRATION_FIELD; i++ )
	{
		if( ( mDeviceNet.mArbitrationField & mask ) != 0 )
			mDeviceNetSimulationData.TransitionIfNeeded( BIT_HIGH );
		else
			mDeviceNetSimulationData.TransitionIfNeeded( BIT_LOW );

		mDeviceNetSimulationData.Advance( samples_per_bit );
		mask = mask >> 1;
	}

	mDeviceNetSimulationData.TransitionIfNeeded( BIT_HIGH ); //we need to end high

	//lets pad the end a bit for the stop bit:
	//mDeviceNetSimulationData.Advance( samples_per_bit );
}

void DeviceNetSimulationDataGenerator::AddCrc()
{
	mFakeStuffedBits.insert(mFakeStuffedBits.end(), mFakeStartOfFrameField.begin(), mFakeStartOfFrameField.end());
	mFakeStuffedBits.insert(mFakeStuffedBits.end(), mFakeArbitrationField.begin(), mFakeArbitrationField.end());
	mFakeStuffedBits.insert(mFakeStuffedBits.end(), mFakeControlField.begin(), mFakeControlField.end());
	mFakeStuffedBits.insert(mFakeStuffedBits.end(), mFakeDataField.begin(), mFakeDataField.end());

	U32 bits_for_crc = mFakeStuffedBits.size();
	U16 crc = ComputeCrc(mFakeStuffedBits, bits_for_crc);
	U32 mask = 0x4000;
	for (U32 i = 0; i < 15; i++)
	{
		if ((mask & crc) == 0)
			mFakeCrcFieldWithoutDelimiter.push_back(mSettings->Dominant());
		else
			mFakeCrcFieldWithoutDelimiter.push_back(mSettings->Recessive());

		mask >>= 1;
	}

	mFakeStuffedBits.insert(mFakeStuffedBits.end(), mFakeCrcFieldWithoutDelimiter.begin(), mFakeCrcFieldWithoutDelimiter.end());

	//CRC DELIMITER (Standard Format as well as Extended Format)
	//The CRC SEQUENCE is followed by the CRC DELIMITER which consists of a single
	//recessive bit.

	mFakeFixedFormBits.push_back(mSettings->Recessive());
}

U16 DeviceNetSimulationDataGenerator::ComputeCrc(std::vector<BitState>& bits, U32 num_bits)
{
	//note that this is a 15 bit CRC (not 16-bit)

	//CRC FIELD (Standard Format as well as Extended Format)
	//contains the CRC SEQUENCE followed by a CRC DELIMITER.
	//CRC SEQUENCE (Standard Format as well as Extended Format)
	//The frame check sequence is derived from a cyclic redundancy code best suited for
	///frames with bit counts less than 127 bits (BCH Code).
	//In order to carry out the CRC calculation the polynomial to be divided is defined as the
	//polynomial, the coefficients of which are given by the destuffed bit stream consisting of
	//START OF FRAME, ARBITRATION FIELD, CONTROL FIELD, DATA FIELD (if
	///present) and, for the 15 lowest coefficients, by 0. This polynomial is divided (the
	//coefficients are calculated modulo-2) by the generator-polynomial:
	//X15 + X14 + X10 + X8 + X7 + X4 + X3 + 1.
	//The remainder of this polynomial division is the CRC SEQUENCE transmitted over the
	//bus. In order to implement this function, a 15 bit shift register CRC_RG(14:0) can be
	//used. If NXTBIT denotes the next bit of the bit stream, given by the destuffed bit
	//sequence from START OF FRAME until the end of the DATA FIELD, the CRC
	//SEQUENCE is calculated as follows:

	//CRC_RG = 0; // initialize shift register
	//REPEAT
	//CRCNXT = NXTBIT EXOR CRC_RG(14);
	//CRC_RG(14:1) = CRC_RG(13:0); // shift left by
	//CRC_RG(0) = 0; // 1 position
	//IF CRCNXT THEN
	//CRC_RG(14:0) = CRC_RG(14:0) EXOR (4599hex);
	//ENDIF
	//UNTIL (CRC SEQUENCE starts or there is an ERROR condition)

	//After the transmission / reception of the last bit of the DATA FIELD, CRC_RG contains
	//the CRC sequence.

	U16 crc_result = 0;
	for (U32 i = 0; i < num_bits; i++)
	{
		BitState next_bit = bits[i];

		//Exclusive or
		if ((crc_result & 0x4000) != 0)
		{
			next_bit = Invert(next_bit);  //if the msb of crc_result is zero, then next_bit is not inverted; otherwise, it is.
		}

		crc_result <<= 1;

		if (next_bit == mSettings->Recessive()) //normally bit high.
			crc_result ^= 0x4599;
	}

	return crc_result & 0x7FFF;
}

void DeviceNetSimulationDataGenerator::WriteFrame(bool error)
{
	U32 recessive_count = 0;
	U32 dominant_count = 0;

	//The frame segments START OF FRAME, ARBITRATION FIELD, CONTROL FIELD,
	//DATA FIELD and CRC SEQUENCE are coded by the method of bit stuffing. Whenever
	//a transmitter detects five consecutive bits of identical value in the bit stream to be
	//transmitted it automatically inserts a complementary bit in the actual transmitted bit
	//stream.

	//The remaining bit fields of the DATA FRAME or REMOTE FRAME (CRC DELIMITER,
	//ACK FIELD, and END OF FRAME) are of fixed form and not stuffed. The ERROR
	//FRAME and the OVERLOAD FRAME are of fixed form as well and not coded by the
	//method of bit stuffing.

	U32 count = mFakeStuffedBits.size();

	if (error == true)
		count -= 9;

	for (U32 i = 0; i < count; i++)
	{

		if (recessive_count == 5)
		{
			mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(1.0));
			recessive_count = 0;
			dominant_count = 1; // this stuffed bit counts

			mDeviceNetSimulationData.Transition(); //to DOMINANT
		}

		if (dominant_count == 5)
		{
			mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(1.0));
			dominant_count = 0;
			recessive_count = 1; // this stuffed bit counts

			mDeviceNetSimulationData.Transition(); //to RECESSIVE
		}

		BitState bit = mFakeStuffedBits[i];

		if (bit == mSettings->Recessive())
		{
			recessive_count++;
			dominant_count = 0;
		}
		else
		{
			dominant_count++;
			recessive_count = 0;
		}

		mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(1.0));
		mDeviceNetSimulationData.TransitionIfNeeded(bit);
	}

	if (error == true)
	{
		if (mDeviceNetSimulationData.GetCurrentBitState() != mSettings->Dominant())
		{
			mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(1.0));
			mDeviceNetSimulationData.Transition(); //to DOMINANT
		}

		mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(8.0));

		mDeviceNetSimulationData.Transition(); //to DOMINANT

		return;
	}

	count = mFakeFixedFormBits.size();

	for (U32 i = 0; i < count; i++)
	{
		mDeviceNetSimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(1.0));
		mDeviceNetSimulationData.TransitionIfNeeded(mFakeFixedFormBits[i]);
	}
}