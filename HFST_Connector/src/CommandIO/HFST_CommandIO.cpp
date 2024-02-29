#include "pch.h"
#include "HFST_CommandIO.hpp"
#include "HFST_APICenter.hpp"
#include "TouchDevice/HFST_TouchDevice.hpp"
#include "TouchDevice/HFST_StaticTemplateFunctions.hpp"

namespace HFST
{
	CommandIO::CommandIO(const HFST_API& api, ChipID nChipID)
		: m_Api(api) 
		, m_ChipID(nChipID){
	}

	bool CommandIO::SetCommandReady()
	{
		unsigned char bufferReady = 0x01;
		int ret = m_Api.TTK.WriteI2CReg(&bufferReady, ADDR_MAP::CMDIO_CTRL, 1);
		if ( ret <= 0 )
		{
			return false;
		}

		return true;
	}

	bool CommandIO::GetCommandReady()
	{
		unsigned char bufferFinish = 0xFF;

		int ret = m_Api.TTK.ReadI2CReg( &bufferFinish, ADDR_MAP::CMDIO_CTRL, 1 );
		if ( ret <= 0 )
			return false;

		int nRetry = 10;
		switch ( static_cast<COMMAND_IO::State>(bufferFinish) )
		{
			case COMMAND_IO::State::PROCESSING:
			{
				while (nRetry > 0)
				{
					ret = m_Api.TTK.ReadI2CReg( &bufferFinish, ADDR_MAP::CMDIO_CTRL, 1 );
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
		unsigned char buffer[COMMAND_IO::CMDIO_PACK_SIZE + 2]{0};

		int ret = m_Api.TTK.ReadI2CReg( buffer, ADDR_MAP::CMDIO_PORT, COMMAND_IO::CMDIO_PACK_SIZE + 2 );

		if ( ret <= 0 )
			return false;

		packet.nCmdID = buffer[0];
		packet.nDataSize = buffer[1];
		memcpy_s( packet.Data, COMMAND_IO::CMDIO_PACK_SIZE, &buffer[2], COMMAND_IO::CMDIO_PACK_SIZE );

		return true;
	}

	bool CommandIO::Write_Packet( const CommandIO_Packet& packet )
	{
		unsigned char buffer[COMMAND_IO::CMDIO_PACK_SIZE + 2]{ 0 };

		memcpy_s( buffer, COMMAND_IO::CMDIO_PACK_SIZE + 2, &packet, COMMAND_IO::CMDIO_PACK_SIZE + 2 );

		int ret = m_Api.TTK.WriteI2CReg( buffer, ADDR_MAP::CMDIO_PORT, COMMAND_IO::CMDIO_PACK_SIZE + 2 );

		if (ret < 0)
			return false;

		return true;
	}

	bool CommandIO::Read( MEMORY_TYPE nMemType, int nAddr, unsigned char* buf, int len )
	{
		int nAddress = nAddr;
		if ( (nMemType == MEMORY_TYPE::AFE_MEM || nMemType == MEMORY_TYPE::AFE_REG) && m_ChipID == ChipID::A8018 )
		{
			nAddress = nAddr & 0x0FFF;
		}

		CommandIO_Packet packet;
		int nRemain = len;
		int nPackSize = 0;
		int nOffset = 0;
		while ( nRemain > 0 )
		{
			nPackSize = ( nRemain > 24 ) ? 24 : nRemain;

			packet.nCmdID = static_cast<int>( COMMAND_IO::CmdID::READ );
			packet.nDataSize = 5;
			packet.Data[0] = static_cast<unsigned char>(nMemType);

			if ((nMemType == MEMORY_TYPE::AFE_MEM || nMemType == MEMORY_TYPE::AFE_REG) && m_ChipID == ChipID::A2152)
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
			packet.Data[3] = CalculateChecksum( (unsigned char*)&packet, 6 );

			if ( !Write_Packet(packet) )
				return false;

			if ( !SetCommandReady() )
				return false;

			if ( !GetCommandReady() )
				return false;

			memset( &packet, 0, sizeof(CommandIO_Packet) );
			if ( !Read_Packet(packet) )
				return false;

			nRemain -= nPackSize;
		}
	}
	bool CommandIO::Write( MEMORY_TYPE nMemType, int nAddr, unsigned char* buf, int len )
	{

	}
}
