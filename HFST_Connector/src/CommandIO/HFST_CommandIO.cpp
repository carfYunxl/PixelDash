#include "pch.h"
#include "HFST_CommandIO.hpp"
#include "HFST_APICenter.hpp"

namespace HFST
{
	CommandIO::CommandIO(const HFST_API& api)
		: m_Api(api) {

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
		switch ( bufferFinish )
		{
			case COMMAND_IO::State::PROCESSING:
			{
				while (nRetry > 0)
				{
					ret = m_Api.TTK.ReadI2CReg( &bufferFinish, ADDR_MAP::CMDIO_CTRL, 1 );
					if ( ret <= 0 )
						return false;

					if (bufferFinish == COMMAND_IO::State::OK)
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
		memcpy( packet.Data, &buffer[2], COMMAND_IO::CMDIO_PACK_SIZE );

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

	void CommandIO::Read(int type, int address, unsigned char* buf, int len, unsigned char ChipId)
	{

	}
	void CommandIO::Write(int type, int address, unsigned char* buf, int len, unsigned char ChipId)
	{

	}
}
