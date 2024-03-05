#include "pch.h"
#include "HFST_CommandIO.hpp"
#include "HFST_APICenter.hpp"
#include "HFST_TouchDevice.hpp"
#include "HFST_StaticTemplateFunctions.hpp"

namespace HFST
{
	CommandIO::CommandIO(IC_Info& info)
		: m_Info( info )
	{
	}

	bool CommandIO::SetCommandReady()
	{
		auto api = HFST_API::GetAPI();
		if (!api)
			return false;

		unsigned char bufferReady = 0x01;
		int ret = api->TTK.WriteI2CReg(&bufferReady, ADDR_MAP::CMDIO_CTRL, 1);
		if ( ret <= 0 )
		{
			return false;
		}

		return true;
	}

	bool CommandIO::GetCommandReady()
	{
		unsigned char bufferFinish = 0xFF;

		auto api = HFST_API::GetAPI();
		if (!api)
			return false;

		int ret = api->TTK.ReadI2CReg( &bufferFinish, ADDR_MAP::CMDIO_CTRL, 1 );
		if ( ret <= 0 )
			return false;

		int nRetry = 10;
		switch ( static_cast<COMMAND_IO::State>(bufferFinish) )
		{
			case COMMAND_IO::State::PROCESSING:
			{
				while (nRetry > 0)
				{
					ret = api->TTK.ReadI2CReg( &bufferFinish, ADDR_MAP::CMDIO_CTRL, 1 );
					if ( ret <= 0 )
						return false;

					if ( static_cast<COMMAND_IO::State>(bufferFinish) == COMMAND_IO::State::OK )
						break;

					nRetry--;
				}
				break;
			}
			case COMMAND_IO::State::UNKNOWN_ID:
			case COMMAND_IO::State::CHECKSUM_ERROR:
			{
				return false;
			}
			case COMMAND_IO::State::OK:
			{
				break;
			}
		}

		if ( nRetry <= 0 )
			return false;

		return true;
	}

	bool CommandIO::Read_Packet( CommandIO_Packet& packet )
	{
		auto api = HFST_API::GetAPI();
		if (!api)
			return false;

		unsigned char buffer[COMMAND_IO::CMDIO_PACK_SIZE + 2]{0};

		int ret = api->TTK.ReadI2CReg( buffer, ADDR_MAP::CMDIO_PORT, COMMAND_IO::CMDIO_PACK_SIZE + 2 );

		if ( ret <= 0 )
			return false;

		packet.nCmdID = buffer[0];
		packet.nDataSize = buffer[1];
		memcpy_s( packet.Data, COMMAND_IO::CMDIO_PACK_SIZE, &buffer[2], COMMAND_IO::CMDIO_PACK_SIZE );

		return true;
	}

	bool CommandIO::Write_Packet( const CommandIO_Packet& packet )
	{
		auto api = HFST_API::GetAPI();
		if (!api)
			return false;

		unsigned char buffer[COMMAND_IO::CMDIO_PACK_SIZE + 2]{ 0 };

		memcpy_s( buffer, COMMAND_IO::CMDIO_PACK_SIZE + 2, &packet, COMMAND_IO::CMDIO_PACK_SIZE + 2 );

		int ret = api->TTK.WriteI2CReg( buffer, ADDR_MAP::CMDIO_PORT, COMMAND_IO::CMDIO_PACK_SIZE + 2 );

		if (ret < 0)
			return false;

		return true;
	}

	int CommandIO::Read( MEMORY_TYPE nMemType, int nAddr, unsigned char* buf, int len )
	{
		int nAddress = nAddr;
		if ( (nMemType == MEMORY_TYPE::AFE_MEM || nMemType == MEMORY_TYPE::AFE_REG) && m_Info.nChipID == static_cast<int>(ChipID::A8018) )
		{
			nAddress = nAddr & 0x0FFF;
		}

		CommandIO_Packet packet;
		int nRemain = len;
		int nPackSize = 0;
		int nOffset = 0;
		int nRetry = 0;
		int nRead = 0;
		int nFlag = 0;
		while ( nRemain > 0 )
		{
			nPackSize = ( nRemain > 24 ) ? 24 : nRemain;

			packet.nCmdID = static_cast<int>( COMMAND_IO::CmdID::READ );
			packet.nDataSize = 5;
			packet.Data[0] = static_cast<unsigned char>(nMemType);

			if ((nMemType == MEMORY_TYPE::AFE_MEM || nMemType == MEMORY_TYPE::AFE_REG) && m_Info.nChipID == static_cast<int>(ChipID::A2152) )
			{
				nAddress = nAddr + ( nOffset / 2 );
			}
			else
			{
				nAddress = nAddr + nOffset;
			}

			packet.Data[1] = (nAddress >> 8) & 0xFF;
			packet.Data[2] = (nAddress & 0xFF);
			packet.Data[3] = nPackSize;
			packet.Data[4] = CalculateChecksum( (unsigned char*)&packet, 6 ) & 0xFF;

			if ( !Write_Packet(packet) )
				return -1;

			if ( !SetCommandReady() )
				return -1;

			if ( !GetCommandReady() )
				return -1;

			memset( &packet, 0, sizeof(CommandIO_Packet) );
			if ( !Read_Packet(packet) )
				return -1;

			if ( packet.nCmdID != COMMAND_IO::CMD_RW_OUTPUT || packet.Data[0] != static_cast<unsigned char>(nMemType) )
			{
				nRetry ++;
				if (nRetry > 10)
					return -1;

				continue;
			}

			// check checksum
			if ( (CalculateChecksum((unsigned char*)&packet, packet.nDataSize + 1) & 0xFF) == packet.Data[packet.nDataSize-1] )
			{
				if ((m_Info.nChipID == static_cast<int>(ChipID::A8018) || m_Info.nChipID == static_cast<int>(ChipID::A2152)) && nMemType == MEMORY_TYPE::AFE_REG)
				{
					if (nFlag == 0)
					{
						memcpy(buf + nRead, &(packet.Data[2]), packet.Data[1]);
						nRead += packet.Data[1];
						if ( nRemain > 24 )
						{
							nRemain -= ( packet.Data[1] - 8 );
						}
						else {
							nRemain -= packet.Data[1];
						}

						nFlag = 1;
					}
					else
					{
						memcpy( buf + nRead, &(packet.Data[10]), packet.Data[1] - 8 );
						nRead += ( packet.Data[1] - 8 );
						if ( nRemain > 16 )
						{
							nRemain -= ( packet.Data[1] - 8 );	//data size
						}
						else 
						{
							nRemain = 0;
						}
					}
				}
				else
				{
					memcpy( buf + nOffset, &packet.Data[2], packet.Data[1] );

					nRemain -= packet.Data[1];
					nOffset += packet.Data[1];
					nRead	+= packet.Data[1];
				}
			}
		}

		return nRead;
	}
	int CommandIO::Write( MEMORY_TYPE nMemType, int nAddr, unsigned char* buf, int len )
	{
		int nAddress = nAddr;
		if ((nMemType == MEMORY_TYPE::AFE_MEM || nMemType == MEMORY_TYPE::AFE_REG) && m_Info.nChipID == static_cast<int>(ChipID::A8018))
		{
			nAddress = nAddr & 0x0FFF;
		}

		int nRemain = len;
		int nPackSize = 0;
		int nOffset = 0;
		int nSet = 0;

		CommandIO_Packet packet;
		while (nRemain > 0)
		{
			nPackSize = (nRemain > 24) ? 24 : nRemain;

			packet.nCmdID = static_cast<int>(COMMAND_IO::CmdID::WRITE);
			packet.nDataSize = nPackSize + 5;
			packet.Data[0] = static_cast<unsigned char>(nMemType);

			if ((nMemType == MEMORY_TYPE::AFE_MEM || nMemType == MEMORY_TYPE::AFE_REG) && m_Info.nChipID == static_cast<int>(ChipID::A2152))
			{
				nAddress = nAddr + (nOffset / 2);
			}
			else
			{
				nAddress = nAddr + nOffset;
			}

			packet.Data[1] = (nAddress >> 8) & 0xFF;
			packet.Data[2] = (nAddress & 0xFF);
			packet.Data[3] = nPackSize;

			memcpy_s(&packet.Data[4], nPackSize, buf + nOffset, nPackSize);
			packet.Data[packet.nDataSize - 1] = CalculateChecksum((unsigned char*)&packet, packet.nDataSize + 1) & 0xFF;

			if (!Write_Packet(packet))
				return -1;

			if (!SetCommandReady())
				return -1;

			if (!GetCommandReady())
				return -1;

			nRemain -= nPackSize;
			nOffset += nPackSize;
			nSet	+= nPackSize;
		}

		return nSet;
	}

	bool CommandIO::GetInfo()
	{
		CommandIO_Packet packet;
		packet.nCmdID = static_cast<int>( COMMAND_IO::CmdID::GET_INFO );
		packet.nDataSize = 2;
		packet.Data[0] = static_cast<int>( COMMAND_IO::INFO_ID::INFO_RAW_LINE );
		packet.Data[1] = CalculateChecksum( (unsigned char*)&packet, packet.nDataSize + 1) & 0xFF;

		if ( !Write_Packet(packet) )
			return false;

		if ( !SetCommandReady() )
			return false;

		if ( !GetCommandReady() )
			return false;

		memset( &packet, 0, sizeof(CommandIO_Packet) );
		if ( !Read_Packet(packet) )
			return false;

		int nRetry = 0;
		while ( 1 )
		{
			if (packet.nCmdID != COMMAND_IO::CMD_INFO_OUTPUT)
				nRetry++;
			else
				break;

			if (nRetry > 5)
				return false;

			memset( &packet, 0, sizeof(CommandIO_Packet) );
			if ( !Read_Packet(packet) )
				return false;
		}

		const int TAGTYPESEL_OFFSET = 5;
		const int MUTUAL_MULTILINEENABLE_OFFSET = 6;
		const int SELF_1_MULTILINEENABLE_OFFSET = 9;
		const int SELF_2_MULTILINEENABLE_OFFSET = 12;
		const int KEY_MULTILINEENABLE_OFFSET = 15;

		m_Info.nValidSelfLen			= packet.Data[0x02];
		m_Info.TagTypeSel				= packet.Data[TAGTYPESEL_OFFSET];
		m_Info.Mutual_Axis				= (packet.Data[MUTUAL_MULTILINEENABLE_OFFSET] >> 7) & 0x01;
		m_Info.Mutual_ChnType			= (packet.Data[MUTUAL_MULTILINEENABLE_OFFSET] >> 6) & 0x01;
		m_Info.Mutual_MultiLineEnable	= packet.Data[MUTUAL_MULTILINEENABLE_OFFSET] & 0x01;
		m_Info.Mutual_MultiLineNum		= packet.Data[MUTUAL_MULTILINEENABLE_OFFSET + 1];
		m_Info.Mutual_MultiNoiseLineNum = packet.Data[MUTUAL_MULTILINEENABLE_OFFSET + 2];
		m_Info.Self_1_Axis				= (packet.Data[SELF_1_MULTILINEENABLE_OFFSET] >> 7) & 0x01;
		m_Info.Self_1_ChnType			= (packet.Data[SELF_1_MULTILINEENABLE_OFFSET] >> 6) & 0x01;
		m_Info.Self_1_MultiLineEnable	= packet.Data[SELF_1_MULTILINEENABLE_OFFSET] & 0x01;
		m_Info.Self_1_MultiLineNum		= packet.Data[SELF_1_MULTILINEENABLE_OFFSET + 1];
		m_Info.Self_1_MultiNoiseLineNum = packet.Data[SELF_1_MULTILINEENABLE_OFFSET + 2];
		m_Info.Self_2_Axis				= (packet.Data[SELF_2_MULTILINEENABLE_OFFSET] >> 7) & 0x01;
		m_Info.Self_2_ChnType			= (packet.Data[SELF_2_MULTILINEENABLE_OFFSET] >> 6) & 0x01;
		m_Info.Self_2_MultiLineEnable	= packet.Data[SELF_2_MULTILINEENABLE_OFFSET] & 0x01;
		m_Info.Self_2_MultiLineNum		= packet.Data[SELF_2_MULTILINEENABLE_OFFSET + 1];
		m_Info.Self_2_MultiNoiseLineNum = packet.Data[SELF_2_MULTILINEENABLE_OFFSET + 2];
		m_Info.Key_Axis					= (packet.Data[KEY_MULTILINEENABLE_OFFSET] >> 7) & 0x01;
		m_Info.Key_ChnType				= (packet.Data[KEY_MULTILINEENABLE_OFFSET] >> 6) & 0x01;
		m_Info.Key_MultiLineEnable		= packet.Data[KEY_MULTILINEENABLE_OFFSET] & 0x01;
		m_Info.Key_MultiLineNum			= packet.Data[KEY_MULTILINEENABLE_OFFSET + 1];
		m_Info.Key_MultiNoiseLineNum	= packet.Data[KEY_MULTILINEENABLE_OFFSET + 2];

		return true;
	}
}
