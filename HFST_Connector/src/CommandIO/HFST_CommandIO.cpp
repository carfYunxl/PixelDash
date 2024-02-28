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

	bool CommandIO::IsCommandFinished()
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

	bool TDU_ReadIOCommand(CommandIoPacket* packet) {
		bool bRet = FALSE;
		int ret;
		unsigned char tmp[A8008_CMD_IO_PKT_SIZE];
		//process commmand
		memset(tmp, 0, A8008_CMD_IO_PKT_SIZE);
		ret = ReadI2CReg(tmp, TDU_A8008_DP_CMD_IO_PORT, A8008_CMD_IO_PKT_SIZE);
		if (ret <= 0) {
			TRACE("TDU_ReadIOCommand: read packet error.\n");
			bRet = FALSE;
		}
		else {
			memcpy((void*)packet, (const void*)tmp, A8008_CMD_IO_PKT_SIZE);
			bRet = TRUE;
		}
		return bRet;
	}

	bool TDU_WriteIOCommand(CommandIoPacket* packet) {
		bool bRet = FALSE;
		int ret;
		unsigned char tmp[A8008_CMD_IO_PKT_SIZE];
		memset(tmp, 0x00, A8008_CMD_IO_PKT_SIZE);
		memcpy((void*)tmp, (const void*)packet, A8008_CMD_IO_PKT_SIZE);

		ret = WriteI2CReg(tmp, TDU_A8008_DP_CMD_IO_PORT, 32);
		if (ret <= 0) {
			TRACE("TDU_WriteIOCommand: write packet error.\n");
			bRet = FALSE;
		}
		else {
			bRet = TRUE;
		}

		return bRet;
	}
}
