#include "pch.h"
#include "TTK_API.hpp"
#include "Core.hpp"

TTK_Communication_API::TTK_Communication_API(CHIP_ID id)
    : m_Chipid(id) {

}

bool TTK_Communication_API::CommunicationModeSelect(int cm)
{
    bool res = true;
    switch (cm) {
    case 0:
        m_CommunicationMode = CommunicationMode::CommunicationMode_TouchLink_Bulk;
        break;
    case 1:
        m_CommunicationMode = CommunicationMode::CommunicationMode_Phone_ADB;
        break;
    case 2:
        m_CommunicationMode = CommunicationMode::CommunicationMode_HID_OverI2C;
        break;
    default:
        res = false;
        break;
    }
    return res;
}

bool TTK_Communication_API::HID_Init(int nProductId, int nVenorId, int nReportId, int nUsagePage)
{
    if (m_HID_Context.hHidHandle)
        return true;

    m_HID_LastError = HFST_HID_SDK_Open(&m_HID_Context.hHidHandle, nProductId, nVenorId, nUsagePage);
    if (HFST_HID_ERROR_OK == m_HID_LastError) {
        HFST_HID_SDK_GetCommunicationMaxDataLength(m_HID_Context.hHidHandle,
            &m_HID_Context.nHidReadMaxSize,
            &m_HID_Context.nHidWriteMaxSize);
        m_HID_Context.nHidReportId = nReportId;
        return true;
    }
    return false;
}

void TTK_Communication_API::HID_UnInit()
{
    HFST_HID_SDK_Close(m_HID_Context.hHidHandle);
    m_HID_Context.hHidHandle = nullptr;
}

int TTK_Communication_API::HID_GetLastErrorCode()
{
    return m_HID_LastError & 0xFFFFFF;
}

char* TTK_Communication_API::HID_GetLastErrorMessage()
{
    HFST_HID_SDK_GetErrorDetailMessage(m_HID_LastError, m_HID_ErrorBuffer, MAX_PATH);
    return m_HID_ErrorBuffer;
}

int TTK_Communication_API::HID_ReadRawdata(unsigned char* rawdata, int nReadSize)
{
    // This is special command
    // direct send 0x0A(report id) to hid fw, fw direct return rawdata
    // hid feature data length is 0x08, so read data length max over 8
    // do this just for speed read raw data
    //if ( !rawdata || nReadSize > 0x56 )
    //if ( !rawdata || nReadSize > 512 )
    if (!rawdata || nReadSize < 0x08)
        return -1;
    if (!m_HID_Context.hHidHandle)
        return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

    std::vector< unsigned char > vecWriteBuf;

    static constexpr int nRawdataReportId = 0x0A;
    //static constexpr int nRawdataMaxLength = 0x50 + 3;  // 0x50: real data length, 3: 2 byte fw return length + 1 byte report id
    int nRawdataMaxLength = nReadSize + 3;

    //unsigned char * tmp = new unsigned char[nRewdataMaxLength];
    memset(rawdata, 0, nRawdataMaxLength);
    rawdata[0] = nRawdataReportId;

    m_HID_LastError = HFST_HID_SDK_Read(m_HID_Context.hHidHandle, rawdata, nRawdataMaxLength);
    if (HFST_HID_ERROR_OK != m_HID_LastError) 
        return ERRORMSG_HID_READ - m_HID_LastError;
    memcpy(&rawdata[0], &rawdata[1], nRawdataMaxLength);
    //rawdata = &rawdata[1];
    //memcpy( &rawdata[0], &tmp[1], nRewdataMaxLength );

    //delete[] tmp;
    return nRawdataMaxLength;
}

/*
 * @brief Open ADB shell to allow communication and alloc resource
 * @param [in]  bAdbRunAsRoot   Determine whether run ADB services as root
 * @param [in]	pAdbFilePath	Absolute adb.exe file full path
 * @param [in]  pExtraService   Extra user define service except open ADB shell
 * @note  -1. Usually this interface only open ADB shell run "adb.exe shell"
 *        -2. If bAdbRunAsRoot is true, it will run "adb.exe root" as first command, such as run "getevent -l" command first must have root authority
 *        -3. If pExtraService is assign, after "adb.exe shell" it will run pExtraService command like "adb.exe pExtraService"
 *            such as there is a user-defined service need run
 */
int TTK_Communication_API::ADB_Init(bool bAdbRunAsRoot, const char* pAdbFilePath, const char* pExtraService)
{
    std::filesystem::path adb_path(pAdbFilePath);
    if (!std::filesystem::exists(adb_path))
        return -1;

    adb_path.remove_filename();

    std::filesystem::path client_file = adb_path / "data\\sitronix_client";
    std::filesystem::path server_file = adb_path / "data\\sitronix_server";
    std::filesystem::path cfg_file = adb_path / "data\\HopingTool_CFG.ini";

    if (!std::filesystem::exists(client_file) || !std::filesystem::exists(server_file) || !std::filesystem::exists(cfg_file))
        return -2;

    std::string push_client("push "), push_server("push "), push_cfg("push ");
    push_client.append(client_file.string()).append(" /data/httu");
    push_server.append(server_file.string()).append(" /data/httu");
    push_cfg.append(cfg_file.string()).append(" /data/httu");

    const char* init_cmd[]{ "shell kill -9 `pgrep sitronix`",

                             "shell mkdir /data/httu",
                             "shell rm /data/httu/sitronix_client",
                             "shell rm /data/httu/sitronix_server",
                             "shell rm /data/httu/HopingTool_CFG.ini",

                             push_client.c_str(),
                             push_server.c_str(),
                             push_cfg.c_str(),

                             "shell chmod 777 /data/httu/sitronix_client",
                             "shell chmod 777 /data/httu/sitronix_server",
                             "shell chmod 777 /data/httu/HopingTool_CFG.ini" };

    std::filesystem::path root = std::filesystem::current_path() / "platform-tools" / "adb.exe";

    for (int idx = 0; idx < _countof(init_cmd); ++idx) {
        HFST_ADB_SDK_RunCmd(root.string().c_str(), init_cmd[idx]);
    }
    system("taskkill /F /FI \"IMAGENAME eq adb.exe\"");
    return HFST_ADB_SDK_Open(&m_hAdbContext, bAdbRunAsRoot, pAdbFilePath, pExtraService);
}
/*
 * @brief Release ADB resource
 */
int TTK_Communication_API::ADB_UnInit()
{
	return HFST_ADB_SDK_Close(m_hAdbContext);
}

std::string TTK_Communication_API::ConvertADB_CMD(const std::vector< unsigned char >& data)
{
	std::string cmd = ADB_CLIENT_CMD;
	for (const unsigned char v : data) {
		char tmp_buf[4]{ 0 };
		snprintf(tmp_buf, 4, "%02X", v);
		cmd.append(tmp_buf);
	}
	return cmd;
}

/**
 * @brief Read adb data from server
 * @param data receive data buffer
 * @param len  the data length of result
 * @return 1 pass < 0 fail
 * @note 1. read cmd have 2 result format
 *		   1.1 [HFST] PASS dataSize data1, data2, ...
 *		   1.2 [HFST] FAIL errCode
 *		 2. write cmd have 2 result format
 *		   2.1 [HFST] PASS
 *		   2.2 [HFST] FAIL errCode
 * if write cmd read data, the param 'data' must nullptr
 */
int TTK_Communication_API::ReadPackage_ADB(unsigned char* data, unsigned int len)
{
	DWORD dwStartTime = GetTickCount();
	int nDataSize = 0;

	constexpr unsigned short ReadSize_ADB = 0x400;			// read size of every adb read
	unsigned char tmp_ReadBuf[ReadSize_ADB]{ 0 };

	CString strRemainReadData = _T("");					// adb data recv from cmd windows by text, so data usually
															// not a complete data, need user manual combination
	do {
		// if success, res is current read data length
		int res = HFST_ADB_SDK_Read(m_hAdbContext, tmp_ReadBuf, sizeof(tmp_ReadBuf));

		if (res <= 0) {
			_cprintf("[ERR-R]\r\n");
			return res;
		}

		// Append last remain data
		CString tmpStrTouchData((char*)tmp_ReadBuf, res);
		if (tmpStrTouchData.IsEmpty()) continue;

		if (!strRemainReadData.IsEmpty()) {
			tmpStrTouchData = strRemainReadData + tmpStrTouchData;
			strRemainReadData = _T("");
		}

		// Valid: [HFST] PASS dataSize 0x1234 0x2345 ...
		//        [HFST] FAIL errcode
		int pos = 0;
		CString tmpStrLineData = tmpStrTouchData.Tokenize(_T("\r\n"), pos);
		if (pos > tmpStrTouchData.GetLength()) {
			// not receive complete data
			strRemainReadData = tmpStrTouchData;
			continue;
		}

		int linePos = 0;

		// parse [HFST] 
		CString tmpStrDataTag = tmpStrLineData.Tokenize(_T(" "), linePos);
		if ("[HFST]" != tmpStrDataTag) {
			// not adb server return data, show message remind we check whether regular
			_cprintf("[Adb-Tag] invalid data: %s\n", (LPSTR)CT2A(tmpStrLineData));
			break;
		}

		// PASS or FAIL
		CString tmpStrResultTag = tmpStrLineData.Tokenize(" ", linePos);
		if (-1 == linePos || tmpStrResultTag.IsEmpty()) {
			_cprintf("[Adb-Result] invalid data: %s\n", (LPSTR)CT2A(tmpStrLineData));
			break;
		}

		// if success, write cmd only have tag & result
		if (("PASS" == tmpStrResultTag) && !data)
			return 1;

		// dataSize or errCode
		CString tmpStrDataSize = tmpStrLineData.Tokenize(" ", linePos).Trim();
		if (-1 == linePos || tmpStrDataSize.IsEmpty()) {
			_cprintf("[Adb-Size] invalid data: %s\n", (LPSTR)CT2A(tmpStrLineData));
			break;
		}

		int nResultSize = _tcstol(tmpStrDataSize, nullptr, 16);

		// if error, data size if error code
		if ("FAIL" == tmpStrResultTag) {
			_cprintf("[Adb-FAIL] data: %s\n", (LPSTR)CT2A(tmpStrLineData));
			return nResultSize;
		}

		// parse data
		do {
			CString strValue = tmpStrLineData.Tokenize(" ", linePos);
			if (-1 == linePos || strValue.IsEmpty()) break;

			data[nDataSize++] = _tcstol(strValue, nullptr, 16);
		} while (1);	// parse line data

		break;
	} while (GetTickCount() - dwStartTime < 500);	// once write must read all data

	if (nDataSize != len)
		return -1;
}

int TTK_Communication_API::ReadFlash_ADB_Private(unsigned char* data, unsigned int addr, unsigned int len)
{
	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.push_back(SWISP_Operation);				// read cmd
	vecWriteBuf.push_back(0x05);						// package length low
	vecWriteBuf.push_back(0x00);						// package length high
	vecWriteBuf.push_back(ADB_SW_ISP_READ);				// adb SW Flash read
	vecWriteBuf.push_back(addr & 0xFF);					// read address low
	vecWriteBuf.push_back((addr >> 8) & 0xFF);			// read address high
	vecWriteBuf.push_back(len & 0xFF);					// read length low
	vecWriteBuf.push_back((len >> 8) & 0xFF);			// read length high
	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());
	if (res < 0) return res;

	std::filesystem::path sPath = std::filesystem::current_path();
	std::string cmd("pull /data/httu/flash.dump ");
	cmd.append(sPath.string());

	std::filesystem::path adbPath = sPath / "platform-tools" / "adb.exe";
	sPath /= "flash.dump";

	// check every 0.5 seconds to see whether pull flash.dump success
	int nWaitTime = 0;
	while (!std::filesystem::exists(sPath))
	{
		res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());
		if (HFST_ADB_ERROR_OK != res) {
			_cprintf("[ERR-%d] W1 %s\r\n", res, cmd.c_str());
			return res;
		}
		Sleep(500);
		nWaitTime++;
		_cprintf("[read] waiting for read finish, now cost: %.1fs\r\n", nWaitTime * 0.5);

		if (nWaitTime == 40) return ERRORMSG_ADB_READ_TIMEOUT;
	};

	std::fstream ifs(sPath, std::ios::in | std::ios::binary);
	if (!ifs.is_open())
	{
		return ERRORMSG_ADB_FILE_OPEN_FILE;
	}

	ifs.seekg(0, std::ios::end);
	size_t size = ifs.tellg();
	if (size == -1)
	{
		return ERRORMSG_ADB_READ_LENGTH_ERROR;
	}
	ifs.seekg(0, std::ios::beg);
	ifs.read((char*)data, size);
	ifs.close();

	// delete temporary file : flash.dump
	std::filesystem::remove_all(sPath);

	//// delete temporary file of adb: /data/httu/flash.dump
	cmd = "shell rm //data//httu//flash.dump";
	res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}
	res = ReadPackage_ADB(nullptr, len);
	return res;
}

int TTK_Communication_API::WriteFlash_ADB_Private(unsigned char* data, unsigned int addr, unsigned int len)
{
	std::filesystem::path sPath = std::filesystem::current_path() / "flash.dump";
	if (std::filesystem::exists(sPath))
		std::filesystem::remove_all(sPath);

	std::ofstream ofs(sPath, std::ofstream::out | std::ofstream::binary);
	if (!ofs.is_open())
	{
		return ERRORMSG_ADB_FILE_OPEN_FILE;
	}
	ofs.write((const char*)data, len);
	ofs.close();

	std::string cmd("push ");
	cmd.append(sPath.string());
	cmd.append(" /data/httu/ ");

	std::filesystem::path adbPath = std::filesystem::current_path() / "platform-tools" / "adb.exe";
	int res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());

	// 这里要判断文件是否已经推送到整机，再给整机下CMD
	Sleep(30);

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.push_back(SWISP_Operation);				// Flash R/W cmd
	vecWriteBuf.push_back(0x05);						// package length low
	vecWriteBuf.push_back(0x00);						// package length high
	vecWriteBuf.push_back(ADB_SW_ISP_WRITE);			// adb SW Flash write
	vecWriteBuf.push_back(addr & 0xFF);					// write address low
	vecWriteBuf.push_back((addr >> 8) & 0xFF);			// write address high
	vecWriteBuf.push_back(len & 0xFF);					// write length low
	vecWriteBuf.push_back((len >> 8) & 0xFF);			// write length high

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	if (std::filesystem::exists(sPath))
		std::filesystem::remove_all(sPath);

	//delete temporary file of adb: /data/httu/flash.dump
	cmd = "shell rm //data//httu//flash.dump";
	res = HFST_ADB_SDK_RunCmd(adbPath.string().c_str(), cmd.c_str());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(nullptr, len);
	return res;
}

int TTK_Communication_API::ReadFlashSW_ADB(unsigned char* data, unsigned int addr, unsigned int len)
{
	if (!m_hAdbContext)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;
	else if (!data)
		return ERRORMSG_DATA_NULL;

	if (addr < 0) return ERRORMSG_DATA_NULL;

	unsigned int maxSize = 0;
	int res = 0;

	switch (m_Chipid)
	{
	case CHIP_ID::A8008:
		maxSize = 0x4000;
		break;
	case CHIP_ID::A8010:
		maxSize = 0x10000;
		break;
	case CHIP_ID::A8015:
		maxSize = 0x8400;
		break;
	case CHIP_ID::A2152:
		maxSize = 0x20000;
		break;
	case CHIP_ID::A8018:
		maxSize = 0xFC00;
		break;
	}
	int maxLen = addr + len;

	if (maxLen > maxSize)
		return ERRORMSG_DATA_NULL;

	if (m_Chipid == CHIP_ID::A8018)
	{
		if (maxLen <= Flash48k)
		{
			res = ReadFlash_ADB_Private(data, addr, len);
		}
		else if (maxLen > Flash48k && maxLen <= Flash60k) //如果读取的Flash区间包含了invalid area,则要特别处理
		{
			res = ReadFlash_ADB_Private(data, addr, Flash48k - addr);
			memset(&data[Flash48k - addr], 0xFF, maxLen - Flash48k);//填充0xFF
		}
		else
		{
			if (addr < Flash48k)
			{
				res = ReadFlash_ADB_Private(data, addr, Flash48k - addr);
				memset(&data[Flash48k - addr], 0xFF, Flash60k - Flash48k);//填充0xFF
				res = ReadFlash_ADB_Private(&data[Flash60k - addr], Flash60k, maxLen - Flash60k);
			}
			else if (addr < Flash60k)
			{
				memset(data, 0xFF, Flash60k - addr);//填充0xFF
				res = ReadFlash_ADB_Private(&data[Flash60k - addr], Flash60k, maxLen - Flash60k);
			}
			else
			{
				res = ReadFlash_ADB_Private(data, addr, len);
			}
}
	}
	else
		res = ReadFlash_ADB_Private(data, addr, len);

	return res;
}

int TTK_Communication_API::WriteFlash_ADB(unsigned char* data, unsigned int addr, unsigned int len)
{
	if (!m_hAdbContext)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;
	if (!data || len <= 0)
		return ERRORMSG_DATA_NULL;

	int nMaxFlashSize = 0;
	int nPageSize = 0x400;
	int res = 0;
	switch (m_Chipid)
	{
	case CHIP_ID::A8008:
		nMaxFlashSize = Flash16k;
		break;
	case CHIP_ID::A8010:
		nMaxFlashSize = Flash64k;
		break;
	case CHIP_ID::A8015:
		nMaxFlashSize = Flash33k;
		break;
	case CHIP_ID::A2152:
		nMaxFlashSize = Flash128k;
		nPageSize = PageSize4K;
		break;
	case CHIP_ID::A8018:
		nMaxFlashSize = Flash63k;
		break;
	}

	if (len > Flash61k)
		return ERRORMSG_DATA_NULL;

	if ((addr % nPageSize) || (len % nPageSize))
		return ERRORMSG_DATA_NULL;

	res = WriteFlash_ADB_Private(data, addr, len);

	return res;
}

int TTK_Communication_API::ReadI2CReg_ProtocolA_ADB(unsigned char* data, unsigned int addr, unsigned int len)
{
	if (!m_hAdbContext || len > MAX_ADB_PKT_SIZE)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back(Bridge_T_Read_Reg);				// read cmd
	vecWriteBuf.push_back(0x03);							// package length low
	vecWriteBuf.push_back(0x00);							// package length high
	vecWriteBuf.push_back(addr & 0xFF);					// read address
	vecWriteBuf.push_back(len & 0xFF);					// read length low
	vecWriteBuf.push_back(len >> 8);						// read length high

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W1 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(data, len);
	if (res < 0)
	{
		for (int i = 0; i < 3; ++i)
		{
			res = ReadPackage_ADB(data, len);
			if (res > 0)
			{
				break;
			}
		}

	}
	return res;
}

int TTK_Communication_API::WriteI2CReg_ProtocolA_ADB(unsigned char* data, unsigned int addr, unsigned int len)
{
	if (!m_hAdbContext || len > MAX_ADB_PKT_SIZE)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	int nWriteSize = len + 1;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back(Bridge_T_Write_Reg);    // write command
	vecWriteBuf.push_back(nWriteSize & 0xFF);		// write data size
	vecWriteBuf.push_back(nWriteSize >> 8);		// address high
	vecWriteBuf.push_back(addr & 0xFF);			// address low

	// copy write data
	for (int i = 0; i < len; ++i)
		vecWriteBuf.push_back(data[i]);

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W2 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(nullptr, len);
	return res;
}

int TTK_Communication_API::WriteCmd_ADB(unsigned char* data, unsigned short len)
{
	if (!m_hAdbContext || len > MAX_ADB_PKT_SIZE)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back(Bridge_T_Write_CMD);    // write command
	vecWriteBuf.push_back(len & 0xFF);			// write data length low
	vecWriteBuf.push_back(len >> 8);				// write data length high

	// copy write data
	for (int i = 0; i < len; ++i)
		vecWriteBuf.push_back(data[i]);

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W3 \r\n", res);
		return res;
	}

	res = ReadPackage_ADB(nullptr, len);
	return res;
}

int TTK_Communication_API::ReadCmd_ADB(unsigned char* data, unsigned short len)
{
	if (!m_hAdbContext || len > MAX_ADB_PKT_SIZE)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf;
	vecWriteBuf.clear();

	vecWriteBuf.push_back(Bridge_T_Read_CMD);				// read cmd
	vecWriteBuf.push_back(0x02);							// package length low
	vecWriteBuf.push_back(0x00);							// package length high
	vecWriteBuf.push_back(len & 0xFF);					// read length low
	vecWriteBuf.push_back(len >> 8);						// read length high

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W4 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(data, len);
	return res;
}

/*
 * @brief Notify server automatic record raw data file
 * @param [in] ctx	user define what kink of data to read
 */
int TTK_Communication_API::ADB_StartRecordRawData(FM_ReadRawdata* ctx)
{
	if (!m_hAdbContext || !ctx)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf{ 0x56, 0x00, 0x00, 0x00 };

	vecWriteBuf.push_back(ctx->type);  // interface type
	vecWriteBuf.push_back(ctx->addr);  // read raw data address

	// user want read raw data type, length and times
	for (const auto& data : ctx->readRawdataType) {
		vecWriteBuf.push_back(data.dataType);      // raw data type, typical mutual(04), self(06) ...
		vecWriteBuf.push_back(data.dataLength);    // raw data type length
		vecWriteBuf.push_back(data.dataNum);       // raw data type read times
	}

	vecWriteBuf.push_back(ctx->defaultReadLength); // undefined raw data type length to read
	unsigned int tmpValidDataSize = vecWriteBuf.size() - 3; // exclude command size

	vecWriteBuf[1] = (tmpValidDataSize & 0xFF);
	vecWriteBuf[2] = ((tmpValidDataSize & 0xFF00) >> 8);

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W5 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(nullptr, 0);
	return res;
}

/*
 * @brief Notify server stop record raw data
 * @param [in] pAdbFilePath	Absolute adb.exe file full path
 * @param [in] pSavePath	The path of pc that save server temporary record raw data file
 * @note after stop record raw data, we must pull raw data file to pc and modify format if necessary
 */
int TTK_Communication_API::ADB_StopRecordRawData(const char* pAdbFilePath, const char* pSavePath)
{
	if (!pAdbFilePath || !m_hAdbContext || !pSavePath)
		return ERRORMSG_HID_READ_DEVICE_NOT_OPEN;

	std::vector< unsigned char > vecWriteBuf{ 0x56, 0x01, 0x00, 0x01 };

	std::string cmd_buf = ConvertADB_CMD(vecWriteBuf);
	int res = HFST_ADB_SDK_Write(m_hAdbContext, cmd_buf.c_str(), cmd_buf.length());

	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d] W6 %s\r\n", res, cmd_buf.c_str());
		return res;
	}

	res = ReadPackage_ADB(nullptr, 0);

	std::string cmd("pull /data/httu/RawData.txt ");
	cmd.append(pSavePath);

	res = HFST_ADB_SDK_RunCmd(pAdbFilePath, cmd.c_str());
	if (HFST_ADB_ERROR_OK != res) {
		_cprintf("[ERR-%d]Pull raw data error\n", res);
		return res;
	}

	std::string remove_file_buf("rm /data/httu/RawData.txt");
	HFST_ADB_SDK_Write(m_hAdbContext, remove_file_buf.c_str(), remove_file_buf.length());

	return res;
}

int TTK_Communication_API::ChecksumCalculation(unsigned short* pChecksum, unsigned char* pInData, unsigned long nLen)
{
	unsigned short nChecksum = 0;
	unsigned char lowByteChecksum = 0;
	for (unsigned long i = 0; i < nLen; ++i) {
		nChecksum += pInData[i];
		lowByteChecksum = nChecksum & 0xFF;
		lowByteChecksum = (lowByteChecksum >> 7) | (lowByteChecksum << 1);
		nChecksum = (nChecksum & 0xFF00) | lowByteChecksum;
	}

	if (NULL != pChecksum)
		*pChecksum = nChecksum;

	return nChecksum;
}

void TTK_Communication_API::SleepInProgram(int nSleep)
{
	LARGE_INTEGER ticksPerSecond, start_tick, end_tick;
	QueryPerformanceFrequency(&ticksPerSecond); // CPU –禲碭 ticks 
	QueryPerformanceCounter(&start_tick);
	LONGLONG diff = 0;
	do
	{
		QueryPerformanceCounter(&end_tick); // 挡璸计竚 
		diff = (LONGLONG)(end_tick.QuadPart - start_tick.QuadPart) * (LONGLONG)1000000 / ticksPerSecond.QuadPart;
		diff /= 1000;
		if ((diff) > nSleep) {
			break;
		}
	} while (1);
}

int TTK_Communication_API::WriteUsbAutoRetry(unsigned char* data, unsigned int ilen, unsigned int& rtlen)
{

	int ret;
	int retry = 10;
	while (retry--) {
		ret = WriteUsb(data, ilen, rtlen);
		if (ret > 0)	retry = 0;
		SleepInProgram(nSleepTime);
	}
	if (ret < 0)
		return ERRORMSG_USB_RETRY_TIMEOUT_ERROR;
	if (rtlen != ilen) {
		return ERRORMSG_NON_COMPLETE_TRANSFER;
	}
	return ret;
}

bool TTK_Communication_API::ExWriteFlashPage_Bulk_A8010(unsigned short Addr, unsigned char* Data)
{
	unsigned char RPage[PageSize1K] = { 0 };
	unsigned char WBuffer[BulkLen] = { 0 };
	BOOL fFirstRun = true;

	unsigned char ChecksumR = 0, ChecksumW = 0;
	char StrShow[100];
	unsigned short tempAddr = Addr;
	unsigned short	curPageSize = 1024;
	unsigned short	curTimes = 0;
	int ret, times = 0, i = 0, Windex = 0;
	curTimes = 8;
	if ((Addr & 0xFF) != 0) {
		return false;
	}
    
	for (int j = 0; j < 8; j++) {
		WBuffer[0] = Bridge_T_HWISP;
		WBuffer[1] = 0x86; //Length L 
		WBuffer[2] = 0x00; //Length H        
		WBuffer[3] = HWISP_WriteBySection;
		if (m_Chipid == CHIP_ID::A8010)
		{
			WBuffer[4] = ((Addr + (j << 7)) >> 8);
			WBuffer[5] = (Addr + (j << 7));
		}
		WBuffer[6] = 0;
		if ((Addr) >= Flash63k) {
			WBuffer[6] = 0x80;
		}
		if (m_Chipid == CHIP_ID::A8010)
		{
			WBuffer[7] = 4;
			WBuffer[8] = 0x20;
		}

		for (i = 9; i < BulkLen; i++, Windex++) {
			WBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
		if (ret < 0) {
			return false;
		}
		if (ret != BulkLen) {
			return false;
		}
		memset(WBuffer, 0x00, BulkLen);
		WBuffer[0] = Bridge_T_Continuous;
		for (i = 1; i < BulkLen; i++, Windex++) {
			WBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
		memset(WBuffer, 0x00, BulkLen);
		WBuffer[0] = Bridge_T_Continuous;
		for (i = 1; i <= 10; i++, Windex++) {
			WBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
		if (ret < 0)
		{
			return false;
		}
		if (ret != BulkLen)
		{
			return false;
		}
		tempAddr += (BulkLen * 2);
		times++;
	}
	fFirstRun = false;
	return true;
}

bool TTK_Communication_API::CheckST1801_SPI_FLASH_Busy()
{
	BYTE EBuffer[BulkLen] = { 0 }, RData[BulkLen] = { 0 }, FlashStatus = 0x00, TimeOut = 500;
	int ret = 0;
	do {
		//Read Flash Status
		if (fSPIMode) {
			EBuffer[0] = Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
			EBuffer[1] = 9;		//Length L
			EBuffer[2] = 0;		//Length H
			EBuffer[3] = 0;		//Delay1
			EBuffer[4] = 0;		//Delay2
			EBuffer[5] = 0x80;	//Signature Number
			EBuffer[6] = 0;		//Transfer Method
			EBuffer[7] = 1;		//Read Length L
			EBuffer[8] = 0;		//Read Length H
			EBuffer[9] = 0;		//Reserved
			EBuffer[10] = 0;	//Reserved
			EBuffer[11] = 0x05;	//Read Status
		}
		else {
			EBuffer[0] = 0x14;	//I2C ICP Cmd
			EBuffer[1] = 0x04;	//CMD Len L
			EBuffer[2] = 0x00;	//CMD Len H

			EBuffer[3] = 0x06;		//I2C ICP Read Status Command.
			EBuffer[4] = 0x01;		//Read data length. (L)
			EBuffer[5] = 0x00;	    //Read data length. (H)
			EBuffer[6] = 0x05;		// Write Command.
			EBuffer[7] = 0x00;		// Write Command.

		}
		ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
		if (ret < 0) {
			return false;
		}
		ret = USBComm_ReadFromBulkEx(RData, 64);
		if (ret < 0) {
			return false;
		}
		if (fSPIMode)
			FlashStatus = RData[5];
		else
			FlashStatus = RData[4];
#ifdef _DEBUG
		CString strTmp;
		strTmp.Format("Flash Status = 0x%02X\r\n", FlashStatus);
		OutputDebugString(strTmp);
#endif // _DEBUG
		if (TimeOut == 0) {
			return false;
		}
		else {
			TimeOut--;
			//             SleepInProgram(nSleepTime);
			//             if(nSleepTime<5){
			//                 nSleepTime++;
			//             }
		}
	} while (FlashStatus & 0x01);
	return true;
}

//-------------------------------------------------------------------------------
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data0|~|Data56|-----57bytes
//|0x7F|Data57 |~|Data63|--------------------------------------------------7bytes
//Address + 64
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data64|~|Data120|---57bytes
//|0x7F|Data121|~|Data127|-------------------------------------------------7bytes
//Address + 64
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data128|~|Data184|---57bytes
//|0x7F|Data185|~|Data191|-------------------------------------------------7bytes
//Address + 64
//|0x10|0x68|0x00|0x01|AddressL|AddressH|16K|0x00|0x00|Data192|~|Data248|---57bytes
//|0x7F|Data249|~|Data255|-------------------------------------------------7bytes
//-------------------------------------------------------------------------------
bool TTK_Communication_API::ExWriteFlashPage_Bulk_ST1801(unsigned short Addr, unsigned char* Data)
{
	BYTE EBuffer[BulkLen] = { 0 };
	int ret = 0, times = 0, i = 0, Windex = 0;;
	//Set Write Enable
	EBuffer[0] = 0x75;	//SPI Write
	EBuffer[1] = 3;		//Length L
	EBuffer[2] = 0;		//Length H

	EBuffer[3] = 0;		//Delay1
	EBuffer[4] = 0;		//Delay2

	EBuffer[5] = 0x06;	//Write Enable

	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	if (pFlashWritePageSize == 0x20) {
		//Page Program Command
		EBuffer[0] = 0x7A;	//SPI Write
		EBuffer[1] = 0x28;		//Length L 6
		EBuffer[2] = 0x00;//1;		//Length H 1
		EBuffer[3] = 0;		//Delay1 Td
		EBuffer[4] = 0;		//Delay2 Tw
		EBuffer[5] = 0;		//Delay2 Tp
		EBuffer[7] = 0x02;	//Page Program
		EBuffer[8] = (unsigned char)(Addr >> 16);	//A23-A16
		EBuffer[9] = (unsigned char)(Addr >> 8);	//A15-A8
		EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
		for (i = 0x0b; i < 32 + 0x0b; i++, Windex++) {
			EBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
		if (ret < 0) {
			return false;
		}
	}
	else {
		//Page Program Command
		EBuffer[0] = 0x7A;	//SPI Write
		EBuffer[1] = 0x08;		//Length L 6
		EBuffer[2] = 0x01;//1;		//Length H 1
		EBuffer[3] = 0;		//Delay1 Td
		EBuffer[4] = 0;		//Delay2 Tw
		EBuffer[5] = 0;		//Delay2 Tp
		EBuffer[7] = 0x02;	//Page Program
		EBuffer[8] = (unsigned char)(Addr >> 16);	//A23-A16
		EBuffer[9] = (unsigned char)(Addr >> 8);	//A15-A8
		EBuffer[0x0a] = (unsigned char)(Addr);		//A7-A0
		for (i = 0x0b; i < BulkLen; i++, Windex++) {
			EBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
		if (ret < 0) {
			return false;
		}
		memset(EBuffer, 0x00, BulkLen);
		EBuffer[0] = Bridge_T_Continuous;
		do {
			for (i = 1; i < BulkLen; i++, Windex++) {
				EBuffer[i] = Data[Windex];
			}
			ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
			if (ret < 0) {
				return false;
			}
			if (ret != BulkLen) {
				return false;
			}
		} while (Windex < 256);
	}

	if (CheckST1801_SPI_FLASH_Busy() == false) return false;
	return true;
}

bool TTK_Communication_API::ExWriteFlashPage_Bulk(unsigned short Addr, unsigned char* Data)
{
	switch (m_Chipid)
	{
		case CHIP_ID::A8010:
			return ExWriteFlashPage_Bulk_A8010(Addr, Data);
		case CHIP_ID::A2152:
			return ExWriteFlashPage_Bulk_ST1801(Addr, Data);
		case CHIP_ID::A8008:
		case CHIP_ID::A8015:
		{
			if(fSPIMode)
				return ExWriteFlashPage_Bulk_ST1801(Addr, Data);
		}
	}


	unsigned char RPage[PageSize1K] = { 0 };
	unsigned char WBuffer[BulkLen] = { 0 };
	BOOL fFirstRun = true;

	unsigned char ChecksumR = 0, ChecksumW = 0;
	char StrShow[100];
	unsigned short tempAddr = Addr;
	unsigned short	curPageSize = 0;
	unsigned short	curTimes = 0;
	int ret, times = 0, i = 0, Windex = 0;
	curPageSize = 1024;
	curTimes = 8;
	if ((Addr & 0xFF) != 0)
	{
		//MsgAdd("Wrong Start Address");
		return false;
	}

	while (times < curTimes)
	{
		WBuffer[0] = Bridge_T_HWISP;
		WBuffer[1] = 0x86; //Length L 
		WBuffer[2] = 0x00; //Length H        
		WBuffer[3] = HWISP_Write;
		WBuffer[4] = (tempAddr) >> 7;
		WBuffer[5] = (tempAddr) & 0x7F;

		if (m_Chipid == CHIP_ID::A8008)
		{
			if (tempAddr >= Flash16k) {
				WBuffer[4] = (tempAddr - Flash16k) >> 7;
				WBuffer[5] = (tempAddr - Flash16k) & 0x7F;
				WBuffer[6] = 0x80;
			}
			else {
				WBuffer[6] = 0;
			}
		}
		else if (m_Chipid == CHIP_ID::A8015)
		{
			if (tempAddr >= Flash32k) {
				WBuffer[4] = (tempAddr - Flash32k) >> 7;
				WBuffer[5] = (tempAddr - Flash32k) & 0x7F;
				WBuffer[6] = 0x80;
			}
			else {
				WBuffer[6] = 0;
			}
		}

		for (i = 9; i < BulkLen; i++, Windex++) {
			WBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
		if (ret < 0) {
			return false;
		}
		if (ret != BulkLen) {
			return false;
		}
		memset(WBuffer, 0x00, BulkLen);
		WBuffer[0] = Bridge_T_Continuous;
		for (i = 1; i < BulkLen; i++, Windex++) {
			WBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
		memset(WBuffer, 0x00, BulkLen);
		WBuffer[0] = Bridge_T_Continuous;
		for (i = 1; i <= 10; i++, Windex++) {
			WBuffer[i] = Data[Windex];
		}
		ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
		if (ret < 0)
		{
			return false;
		}
		if (ret != BulkLen)
		{
			return false;
		}
		tempAddr += (BulkLen * 2);
		times++;
	}
	fFirstRun = false;
	return true;
}

bool TTK_Communication_API::ExReadFlashPage_Bulk(unsigned short Addr, unsigned char* Data)
{

	int ret;
	char StrShow[100];
	unsigned char RBuffer[BulkLen] = { 0 };
	short Retry = RetryTimes;
	unsigned short curPageSize = 0;

	switch (m_Chipid)
	{
	case CHIP_ID::A8008:
		curPageSize = PageSize512;
		break;
	case CHIP_ID::A8010:
		curPageSize = PageSize1K;
		break;
	case CHIP_ID::A8015:
		curPageSize = PageSize512;
		break;
	case CHIP_ID::A2152:
		curPageSize = PageSize1K;
		return ExReadFlashPage_Bulk_SPI_ST1801(Addr, Data);
	}

	if ((Addr & 0xFF) != 0) {
		return false;
	}

	RBuffer[0] = Bridge_T_HWISP;
	RBuffer[1] = 6; //Length L
	RBuffer[2] = 0; //Length H 
	RBuffer[3] = HWISP_Read;

	switch (m_Chipid)
	{
		case CHIP_ID::A8008:
		{
			RBuffer[4] = Addr >> 7;
			RBuffer[5] = Addr & 0x7F;
			if (Addr >= Flash16k)
				RBuffer[6] = 0x80;
			else
				RBuffer[6] = 0;
			break;
		}
		case CHIP_ID::A8010:
		{
			RBuffer[4] = Addr >> 8;
			RBuffer[5] = Addr & 0x8F;
			if (Addr >= Flash63k)
				RBuffer[6] = 0x80;
			else
				RBuffer[6] = 0;
			break;
		}
		case CHIP_ID::A8015:
		{
			RBuffer[4] = Addr >> 7;
			RBuffer[5] = Addr & 0x7F;
			if (Addr >= Flash32k) {
				RBuffer[4] = (Addr - Flash32k) >> 7;
				RBuffer[5] = (Addr - Flash32k) & 0x7F;
				RBuffer[6] = 0x80;
			}
			else
				RBuffer[6] = 0;
			break;
		}
	}

	RBuffer[7] = (curPageSize & 0xFF);; // 256byte
	RBuffer[8] = (curPageSize >> 8);
	while (Retry--)
	{
		ret = USBComm_WriteToBulkEx(RBuffer, BulkLen);
		if (ret < 0)
		{
			//ShowMessage("USB_Write_Fail");
			return false;
		}
		if (ret != BulkLen)
		{
			//ShowMessage("Non-complete");
			return false;
		}

		ret = Read_Packet_Bulk(Data, curPageSize);
		if (((ret < 0) && (ret != ERRORMSG_BRIDGE_STATUS_NAK))) //When status is NAK, it retries several times.
		{
			return false;
		}
		else
			return true;
	}
	if (Retry == 0)
	{
		return false;
	}

	return true;
}

bool TTK_Communication_API::ExReadFlashPage_Bulk_SPI_ST1802(int Addr, unsigned char* Data)
{
	int ret = 0;
	BYTE EBuffer[BulkLen] = { 0 }, RBuffer[BulkLen] = { 0 }, OutPutData[PageSize1K * 2] = { 0 };
	//Read Flash Data
	EBuffer[0] = Bridge_Read_SPI_AvdRead_Command_Packet;	//SPI Read
	EBuffer[1] = 12;	//Length L
	EBuffer[2] = 0;		//Length H

	EBuffer[3] = 0;		//Delay1 Td 0
	EBuffer[4] = 0;		//Delay2 Tr 1
	EBuffer[5] = 0x80;	//Signature 2
	EBuffer[6] = 0x00;		//Transfer Method 3
	EBuffer[7] = 0;		//Len L 4 
	EBuffer[8] = 4;		//Len H 5
	EBuffer[9] = 0;		//Tr 6
	EBuffer[10] = 0;	// 7

	EBuffer[11] = 0x03;	//Read Data 8
	EBuffer[12] = (unsigned char)(Addr >> 16);	//A23-A16
	EBuffer[13] = (unsigned char)(Addr >> 8);	//A15-A8
	EBuffer[14] = (unsigned char)(Addr);		//A7-A0

	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	int nIndex = 0;
	for (int i = 0; i < 17; i++) {
		ret = USBComm_ReadFromBulkEx(RBuffer, 64);
		if (ret < 0) {
			return false;
		}
		if (i == 0) {
			for (int j = 8; j < 64; j++) {
				OutPutData[nIndex++] = RBuffer[j];
			}
		}
		else {
			for (int j = 1; j < 64; j++) {
				OutPutData[nIndex++] = RBuffer[j];
			}
		}
	}

	memcpy(Data, OutPutData, PageSize1K);
	return true;
}

bool TTK_Communication_API::ExReadFlashPage_Bulk_SPI_ST1801(unsigned short Addr, unsigned char* Data)
{
	int ret = 0;
	BYTE EBuffer[BulkLen] = { 0 }, RBuffer[BulkLen] = { 0 }, OutPutData[PageSize1K * 2] = { 0 };
	//Read Flash Data
	EBuffer[0] = Bridge_Read_SPI_AvdRead_Command_Packet;//Bridge_SPI_AvdRead_Command_Packet;	//SPI Read
	EBuffer[1] = 12;	//Length L
	EBuffer[2] = 0;		//Length H

	EBuffer[3] = 0;		//Delay1
	EBuffer[4] = 0;		//Delay2
	EBuffer[5] = 0x80;	//Signature Number
	EBuffer[6] = 0;		//Transfer Method
	EBuffer[7] = 0;		//Read Length L
	EBuffer[8] = 4;		//Read Length H
	EBuffer[9] = 0;		//Reserved
	EBuffer[10] = 0;	//Reserved

	EBuffer[11] = 0x03;	//Read Data
	EBuffer[12] = (unsigned char)(Addr >> 16);	//A23-A16
	EBuffer[13] = (unsigned char)(Addr >> 8);	//A15-A8
	EBuffer[14] = (unsigned char)(Addr);		//A7-A0

	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	int nIndex = 0;
	for (int i = 0; i < 17; i++) {
		ret = USBComm_ReadFromBulkEx(RBuffer, 64);
		if (ret < 0) {
			return false;
		}
		if (i == 0) {
			for (int j = 8; j < 64; j++) {
				OutPutData[nIndex++] = RBuffer[j];
			}
		}
		else {
			for (int j = 1; j < 64; j++) {
				OutPutData[nIndex++] = RBuffer[j];
			}
		}
	}
	memcpy(Data, OutPutData, PageSize1K);
	return true;
}

int TTK_Communication_API::EraseFlashHW_ST1801(unsigned int Addr)
{
	BYTE EBuffer[BulkLen] = { 0 };
	int ret = 0;

	ret = GetTouchLinkVersion(&g_TouchLinkVersion);
	if (ret <= 0) return ret;

	EBuffer[0] = 0x75;	//SPI Write
	EBuffer[1] = 3;		//Length L
	EBuffer[2] = 0;		//Length H

	EBuffer[3] = 0;		//Delay1
	EBuffer[4] = 0;		//Delay2

	EBuffer[5] = 0x06;	//Write Enable

	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	//Erase Sector Command
	EBuffer[0] = 0x75;	//SPI Write
	EBuffer[1] = 6;		//Length L
	EBuffer[2] = 0;		//Length H

	EBuffer[3] = 0;		//Delay1
	EBuffer[4] = 0;		//Delay2

	EBuffer[5] = 0x20;	//Sector Erase
	EBuffer[6] = (unsigned char)(Addr >> 16);	//A23-A16
	EBuffer[7] = (unsigned char)(Addr >> 8);	//A15-A8
	EBuffer[8] = (unsigned char)(Addr);		//A7-A0

	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	if (CheckST1801_SPI_FLASH_Busy() == false) {
		return false;
	}
	return true;
}

//------------------------------------------------------------------
//|0x10|0x04|0x00|0x00|AddressH|AddressL|16K|
//------------------------------------------------------------------
bool TTK_Communication_API::ExEraseFlashPage_Bulk(unsigned short Addr)
{
	if( m_Chipid == CHIP_ID::A2152 )
		return EraseFlashHW_ST1801(Addr);

	if (m_Chipid == CHIP_ID::A8010)
	{
		if (Addr >= 0xC000 && Addr < 0xFC00) {
			return true;
		}
	}

	int ret;
	char StrShow[100];
	unsigned char EBuffer[BulkLen] = { 0 };

	if ((Addr & 0xFF) != 0) {
		return false;
	}

	EBuffer[0] = Bridge_T_HWISP;
	EBuffer[1] = 4; //Length L
	EBuffer[2] = 0; //Length H
	EBuffer[3] = HWISP_Erase;
	EBuffer[4] = Addr >> 7;
	EBuffer[5] = Addr & 0x7F;
	if (m_Chipid == CHIP_ID::A8008)
	{
		if (Addr >= Flash16k) {
			EBuffer[4] = (Addr - Flash16k) >> 7;
			EBuffer[5] = (Addr - Flash16k) & 0x7F;
			EBuffer[6] = 0x80;
		}
		else {
			EBuffer[6] = 0x00;
		}
	}
	else if (m_Chipid == CHIP_ID::A8015)
	{
		if (Addr >= Flash32k) {
			EBuffer[4] = (Addr - Flash32k) >> 7;
			EBuffer[5] = (Addr - Flash32k) & 0x7F;
			EBuffer[6] = 0x80;
		}
		else {
			EBuffer[6] = 0x00;
		}
	}
	else if (m_Chipid == CHIP_ID::A8010)
	{
		EBuffer[4] = Addr >> 8;
		EBuffer[5] = Addr & 0x8F;
		if (Addr >= Flash63k) {
			EBuffer[4] = Addr >> 8;
			EBuffer[5] = Addr & 0xFF;
			EBuffer[6] = 0x80; //禬筁16k, 璶硄Touch-Link
		}
		else {
			EBuffer[6] = 0;
		}
	}

	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	if (ret != BulkLen) {
		return false;
	}
	return true;
}

//------------------------------------------------------------------
//|0x10|0x04|0x00|0x03|
//------------------------------------------------------------------
bool TTK_Communication_API::MassEraseFlash_Bulk(void)
{
	int ret;
	unsigned char EBuffer[BulkLen] = { 0 };

	EBuffer[0] = Bridge_T_HWISP;
	EBuffer[1] = 4; //Length L
	EBuffer[2] = 0; //Length H
	EBuffer[3] = HWISP_Mass_Erase;
	EBuffer[6] = 0x00; //All erase
	ret = USBComm_WriteToBulkEx(EBuffer, BulkLen);
	if (ret < 0) {
		return false;
	}
	SleepInProgram(40);
	return true;
}

//------------------------------------------------------------------
//ST1572: |0x88|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
//
//|VerIntegral[3]|VerIntegral[2]|VerIntegral[1]|VerIntegral[0]|VerDecimal[3] |VerDecimal[2] |VerDecimal[1] |VerDecimal[0]|
//------------------------------------------------------------------
bool TTK_Communication_API::ReadFWVersion_SW(float* Ver)
{
	unsigned char VBuffer[BulkLen] = { 0 };
	int ret;

	VBuffer[0] = SWISP_Get_FWVer;
	VBuffer[1] = 0;
	VBuffer[2] = 0;
	VBuffer[3] = 0;
	VBuffer[4] = 0;
	VBuffer[5] = 0;
	VBuffer[6] = 0;
	VBuffer[7] = 0;

	ret = WriteCmd(VBuffer, SWISPLength);
	if (ret < 0) {
		return false;
	}
	ret = ReadCmd(VBuffer, SWISPLength);
	if (ret < 0) {
		return false;
	}
	*Ver = (float)VBuffer[7] * 1000.0 + (float)VBuffer[6] * 100.0 + (float)VBuffer[5] * 10.0 + (float)VBuffer[4] * 1.0 + \
		(float)VBuffer[3] * 0.1 + (float)VBuffer[2] * 0.01 + (float)VBuffer[1] * 0.001 + (float)VBuffer[0] * 0.0001;
	return true;
}

//------------------------------------------------------------------
//ST1572: |0x8B|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
//
//|ChecksumL|ChecksumH|0x00|0x00|0x00|0x00|0x00|0x00|
//------------------------------------------------------------------
bool TTK_Communication_API::ReadChecksum_SW(unsigned int Length, unsigned int* Checksum)
{
	unsigned char CBuffer[BulkLen] = { 0 };
	int ret;

	CBuffer[0] = SWISP_Read_Checksum;
	CBuffer[1] = Length & 0xFF;
	CBuffer[2] = (Length >> 8) & 0xFF;
	CBuffer[3] = (Length >> 16) & 0xFF;
	CBuffer[4] = (Length >> 24) & 0xFF;
	CBuffer[5] = 0;
	CBuffer[6] = 0;
	CBuffer[7] = 0;

	ret = WriteCmd(CBuffer, SWISPLength);
	if (ret < 0) {
		return false;
	}

	ret = ReadCmd(CBuffer, SWISPLength);
	if (ret < 0) {
		return false;
	}
	*Checksum = CBuffer[0] + (CBuffer[1] << 8);
	return true;
}

//----------------------------------------------------------
bool TTK_Communication_API::Jump_SWISP(void)
{
	int ret, i = 0, Repeat = 2;
	unsigned char state[9];
	ret = ReadI2CReg(state, 0x01, 0x08);
	if (ret < 0)
	{
		return false;
	}

	if (state[0] == 0x06)
	{
		return true;
	}

	unsigned char pattern[9] = { "STX_FWUP" };

	//====check status====end*/
	Repeat = 2;
	while (Repeat--)
	{
		for (i = 0; i < 8; i++)
		{
			//ret = WriteI2CReg_Bulk(0x00,JumpSWISPString[i]);
			ret = WriteI2CReg(&pattern[i], 0x00, 1);
			if (ret == false)
			{
				return false;
			}
		}

		SleepInProgram(300);
		//====check status====
		ret = ReadI2CReg(state, 0x01, 0x08);
		if (ret < 0)
		{
			return false;
		}
		if (state[0] == 0x06)
		{
			return true;
		}
		//====check status====end
		SleepInProgram(nSleepTime);
		//====check status====
		ret = ReadI2CReg(state, 0x01, 0x08);
		if (ret < 0)
		{
			return false;
		}
		if (state[0] == 0x06)
		{
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------
//ST1572: |0x89|0x00|0x00|0x00|0x00|0x00|0x00|0x00|
//------------------------------------------------------------------
bool TTK_Communication_API::JumpBack_SW(void)
{
	unsigned char JBuffer[BulkLen] = { 0 };
	int ret;

	JBuffer[0] = SWISP_Run_APROM;
	JBuffer[1] = 0;
	JBuffer[2] = 0;
	JBuffer[3] = 0;
	JBuffer[4] = 0;
	JBuffer[5] = 0;
	JBuffer[6] = 0;
	JBuffer[7] = 0;

	ret = WriteCmd(JBuffer, SWISPLength);
	if (ret < 0) {
		return false;
	}
	return true;
}

//------------------------------------------------------------------
bool TTK_Communication_API::GetResponse_SW(unsigned short* Checksum)
{
	int ret;
	char StrShow[100];
	unsigned char GBuffer[BulkLen] = { 0 };

	ret = ReadCmd(GBuffer, SWISPLength);
	if (ret < 0)
	{
		return false;
	}
	if (GBuffer[0] != 0x8F)
	{
		return false;
	}
	else {
		if (GBuffer[2] & 0x10)
		{
			return false;
		}
		else {
			if (GBuffer[2] & 0x02)
			{
				return false;
			}
			else {
				if (GBuffer[2] & 0x01)
				{
					//MsgAdd("Flash Error");
					return false;
				}
				else
				{
					//MsgAdd("Flash Correct");
				}
			}
		}
	}
	*Checksum = GBuffer[4] + (GBuffer[5] << 8);
	return true;
}

//------------------------------------------------------------------
//ST1572: |0x8D|0x00|BlockNum|0x00|0x00|0x00|0x00|0x00|
//
//|Data0 |Data1 |Data2 |Data3 |Data4 |Data5 |Data6 |Data7 |
//|Data8 |Data9 |Data10|Data11|Data12|Data13|Data14|Data15|
//...
//|Data504|Data505|Data506|Data507|Data508|Data509|Data510|Data511|
//------------------------------------------------------------------
bool TTK_Communication_API::ReadDataFlashBlock_SW(unsigned int Addr, unsigned char* Data)
{
	unsigned char RBuffer[BulkLen] = { 0 };
	int i, ret, iData = 0;

	RBuffer[0] = SWISP_Read_DataFlash;
	RBuffer[1] = 0;
	RBuffer[2] = (Addr >> 9) & 0xFF;
	RBuffer[3] = 0;
	RBuffer[4] = 0;
	RBuffer[5] = 0;
	RBuffer[6] = 0;
	RBuffer[7] = 0;

	ret = WriteCmd(RBuffer, SWISPLength);
	if (ret < 0) {
		return false;
	}
	while (iData < BlockSize)
	{
		ret = ReadCmd(RBuffer, SWISPLength);
		if (ret < 0) {
			return false;
		}

		for (i = 0; (i < SWISPLength) && (iData < BlockSize); i++, iData++) {
			Data[iData] = RBuffer[i];
		}
	}
	return true;
}

//-----------------------------------------------------------------
bool TTK_Communication_API::ResetBridgeStatus_Bulk(void)
{
	int ret;
	unsigned char WBuffer[BulkLen] = { 0 };
	WBuffer[0] = Bridge_T_ResetBridge;
	WBuffer[1] = 0; //Length L
	WBuffer[2] = 0; //Length H

	ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
	if (ret < 0)
	{
		////ShowMessage("USB_Write_Fail");
		return false;
	}
	if (ret != BulkLen)
	{
		////ShowMessage("Non-complete");
		return false;
	}
	return true;
}

//----------------------------------------------------------------
int TTK_Communication_API::Read_Packet_Bulk(unsigned char* RBuffer, unsigned short RLength)
{
	unsigned char RData[UsbMaxReadLen] = { 0 };
	int ReadL = 0, ret, timeout = ReadTimeout, i = 0;
	int done = false;
	int countFO = 0;
	int usbReadNum;
	DWORD dwStartTick = GetTickCount();
	do
	{
		usbReadNum = UsbMaxReadLen;
		timeout = ReadTimeout;
		done = false;
		int numReadUsbReadBytes = 0;
		int numReadUsbUsedBytes = 0;
		int numReadTime = 0;
		if (ReadL == 0) { // first one
			for (int k = 0; k < UsbMaxReadLen / 60; k++) {
				numReadUsbReadBytes += 64;
				numReadTime++;
				if (k == 0) {
					numReadUsbUsedBytes += 60;
				}
				else {
					numReadUsbUsedBytes += 63;
				}
				if (numReadUsbReadBytes >= UsbMaxReadLen) {
					break;
				}
				if (numReadUsbUsedBytes >= (RLength - ReadL)) {
					break;
				}
			}
		}
		else {
			for (int k = 0; k < 100000; k++) {
				numReadUsbReadBytes += 64;
				numReadTime++;
				numReadUsbUsedBytes += 63;
				if (numReadUsbReadBytes >= UsbMaxReadLen) {
					break;
				}
				if (numReadUsbUsedBytes >= (RLength - ReadL)) {
					break;
				}
			}
		}
		usbReadNum = numReadUsbReadBytes;
		ret = USBComm_ReadFromBulkEx(RData, usbReadNum);
		if (ret < 0)
		{
			return ERRORMSG_BRIDGE_STATUS_TIMEOUT; ;
		}

		int bankNum = ((ret - 1) / 64) + 1;
		int bankCnt = 0;
		if (fSPIMode) {
			//  if(RData[0]==Bridge_R_SPI_Data_Packet && RData[4]==0x87 && RData[5]==0x00){
			if ((RData[0] == Bridge_R_SPI_Data_Packet || RData[0] == Bridge_R_HWISP_Data) || (RData[0] == 0xff)) {

			}
			else {
				return ERRORMSG_READ_PACKET_ERROR;
			}
		}
#if( (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802))
		if (RData[0] != 0x82) {
			//return ERRORMSG_READ_PACKET_ERROR;
		}
#endif
		while ((bankCnt < bankNum) && (ReadL < RLength)) {
			int nErrorTimeOut = 1000;
			do
			{
				switch (RData[0 + bankCnt * 64])
				{
				case Bridge_R_SPI_Data_Packet:
				case Bridge_R_Data:
				case Bridge_R_I2C_Data:
				case Bridge_R_HWISP_Data:
					if (fSPIMode && RData[0] == Bridge_R_SPI_Data_Packet) {
						for (i = 7; (i < 64) && (ReadL < RLength); i++, ReadL++)
						{
							RBuffer[ReadL] = RData[i + (bankCnt * 64)];
						}
					}
					else {
						for (i = 4; (i < 64) && (ReadL < RLength); i++, ReadL++)
						{
							RBuffer[ReadL] = RData[i + (bankCnt * 64)];
						}
					}
					done = true;
					bankCnt++;
					break;
				case Bridge_R_Continuous:
					for (i = 1; (i < 64) && (ReadL < RLength); i++, ReadL++)
					{
						RBuffer[ReadL] = RData[i + (bankCnt * 64)];

					}
					done = true;
					bankCnt++;
					break;
				case Bridge_R_Status:
					countFO++;
					memcpy(&RBuffer[0], &RData[0], RLength);
					switch (RData[4])
					{
					case BridgeStatus_TouchPanel_Detect_Mode:
						return ERRORMSG_BRIDGE_STATUS_SCANNING;
						break;
					case Bridge_Status_Running:
						break;
					case Bridge_Status_Initialization:
						break;
					case Bridge_Status_Ready:
						switch (RData[5])
						{
						case I2C_Status_ACK:
							done = true;
							return ERRORMSG_TIMEOUT;
							break;
						case I2C_Status_Initialization:
							done = true;
							return ERRORMSG_TIMEOUT;
							break;
						case I2C_Status_NAK:
							return ERRORMSG_BRIDGE_STATUS_NAK;
						case I2C_Status_Timeout:
							return ERRORMSG_BRIDGE_STATUS_TIMEOUT;
						case I2C_Status_Arbitration_Loss:
							return ERRORMSG_BRIDGE_STATUS_ARB_Loss;
						case I2C_Status_Bus_Fault:
							return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;
						default:
							return ERRORMSG_BRIDGE_STATUS_ERROR;
							break;
						}

						break;
					case Bridge_Status_Error:
						switch (RData[8])
						{
						case Error_Bulk_Length:
							break;
						case Error_First_Packet:
							break;
						case Error_Continuous_Packet:
							break;
						case Error_Unexpected:
							break;
						case Error_UnknownCmd:
							break;
						case Error_ReadTimeout:
							break;
						case Error_VddGndShort:
							return ERRORMSG_BRIDGE_STATUS_ERROR_VDDGND_Short;
							break;
						}
						ResetBridgeStatus_Bulk();
						return ERRORMSG_BRIDGE_STATUS_ERROR;
					default:
						break;
					}
					break;
				default:
					return ERRORMSG_READ_PACKET_ERROR;
				}
			} while (!done);
		}
		if (timeout <= 0)
		{
			return ERRORMSG_TIMEOUT;
		}
	} while ((ReadL < RLength) && (timeout--) && ((GetTickCount() - dwStartTick) < 3000));

	return 1;
}

//----------------------------------------------------------------
int TTK_Communication_API::Read_Packet_INT_Bulk(unsigned char* RBuffer, unsigned short RLength)
{
	unsigned char RData[BulkLen] = { 0 };
	char StrShow[100];
	int ReadL = 0, ret, i = 0;

	ret = USBComm_ReadFromBulkEx(RData, BulkLen);

	if (ret < 0) {
		return ERRORMSG_READ_BLUK_FAIL;
	}
	if (ret != BulkLen) {
		return ERRORMSG_NON_COMPLETE_TRANSFER;
	}

	memset(RBuffer, 0, sizeof(RBuffer));
	switch (RData[0]) {
	case Bridge_R_INT_Detect_Data_Packet_Mode2:
		for (i = 0; (i < 64) && (ReadL < RLength); i++, ReadL++)
		{
			RBuffer[ReadL] = RData[i];
		}
		return ret;
		break;
	case Bridge_R_INT_Detect_Data_Packet:
		for (i = 4; (i < 64) && (ReadL < RLength); i++, ReadL++)
		{
			RBuffer[ReadL] = RData[i];
		}
		return 1;
		break;
	case Bridge_R_Data:
	case Bridge_R_I2C_Data:
	case Bridge_R_HWISP_Data:
		for (i = 4; (i < 64) && (ReadL < RLength); i++, ReadL++)
		{
			RBuffer[ReadL] = RData[i];
		}
		break;
	case Bridge_R_Continuous:
	case Bridge_R_Status:

		switch (RData[4])
		{
		case BridgeStatus_TouchPanel_Detect_Mode:
		{

		}
		break;
		case Bridge_Status_Running:
			break;
		case Bridge_Status_Initialization:
			break;
		case Bridge_Status_Ready:
			switch (RData[5])
			{
			case I2C_Status_ACK:
				break;
			case I2C_Status_Initialization:
				break;
			case I2C_Status_NAK:
				return ERRORMSG_BRIDGE_STATUS_NAK;
			case I2C_Status_Timeout:
				return ERRORMSG_BRIDGE_STATUS_TIMEOUT;
			case I2C_Status_Arbitration_Loss:
				return ERRORMSG_BRIDGE_STATUS_ARB_Loss;
			case I2C_Status_Bus_Fault:
				return ERRORMSG_BRIDGE_STATUS_BUS_FAULT;
			default:
				break;
			}

			break;
		case Bridge_Status_Error:
			switch (RData[8])
			{
			case Error_Bulk_Length:
				break;
			case Error_First_Packet:
				break;
			case Error_Continuous_Packet:
				break;
			case Error_Unexpected:
				break;
			case Error_UnknownCmd:
				break;
			case Error_ReadTimeout:
				break;
			}
			ResetBridgeStatus_Bulk();
			return ERRORMSG_BRIDGE_STATUS_ERROR;
		default:
			break;
		}
		return -1;  //No Data
	default:
		return ERRORMSG_READ_PACKET_ERROR;
	}
	return 1;
}

int TTK_Communication_API::Read_TouchPanelDetectStatus()
{
	unsigned char RData[BulkLen] = { 0 };
	char StrShow[100];
	int ReadL = 0, ret = 0, i = 0;

	ret = USBComm_ReadFromBulkEx(RData, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_READ_BLUK_FAIL;
	}
	if (ret != BulkLen)
	{
		return ERRORMSG_NON_COMPLETE_TRANSFER;
	}

	if (RData[0] == Bridge_R_INT_Detect_Data_Packet_Mode2) {
		if (RData[0x03] == BridgeStatus_TouchPanel_Detect_Mode) {//0x06 = 
			if (RData[0x04] == 0x01) {
				ret = (RData[5] << 8) + (RData[6] << 4) + RData[7];
			}
			else {
				ret = ERRORMSG_TP_Not_Connect;
			}
		}
	}
	else {
		ret = ERRORMSG_Not_Touch_Panel_Detect_Mode_Status;
	}
	return ret;
}

//-----------------------------------------------------------------
//mode 0: Jump Bulk
//mode 1: Jump HID
//-----------------------------------------------------------------
int TTK_Communication_API::SwitchBridge(unsigned char mode)
{
	int ret;
	unsigned char WBuffer[BulkLen] = { 0 };
	WBuffer[0] = Bridge_T_System_OP;
	WBuffer[1] = 2; //Length L
	WBuffer[2] = 0; //Length H
	WBuffer[3] = 0x0F;  //Bridge Switch
	WBuffer[4] = mode;
	ret = USBComm_WriteToBulkEx(WBuffer, BulkLen);
	if (ret < 0)
	{
		////ShowMessage("USB_Write_Fail");
		return ERRORMSG_WRITE_BLUK_FAIL;
	}
	if (ret != BulkLen)
	{
		////ShowMessage("Non-complete");
		return ERRORMSG_NON_COMPLETE_TRANSFER;
	}
	return true;
}

//------------------------------------------------------------------
//T:|0x6F|0x03|0x00|0x03|LED R/W|LED ON/OFF|
//------------------------------------------------------------------
int TTK_Communication_API::SetLED_Bulk(unsigned char LEDSetting, BOOL fFlashMode, BYTE pPeriod)
{
	unsigned char TBuffer[BulkLen] = { 0 };
	int ret;
	TBuffer[0] = Bridge_T_System_OP;
	if (fFlashMode == false) {
		TBuffer[1] = 0x03;
		TBuffer[2] = 0x00;
		TBuffer[3] = 0x03; //LED Status Control   
		TBuffer[4] = 0x00; //write   Data Buffer[1]
		TBuffer[5] = LEDSetting;  //Data Buffer[2]
		TBuffer[6] = pPeriod;  //Data Buffer[3]
	}
	else {
		if (pPeriod == 0) {
			TBuffer[1] = 0x02;
			TBuffer[2] = 0x00;
			TBuffer[3] = 0x06; //LED Status Control   
			TBuffer[4] = 0x00; //NormalMode   Data Buffer[1]
		}
		else {
			TBuffer[1] = 0x04;
			TBuffer[2] = 0x00;
			TBuffer[3] = 0x06; //LED Status Control   
			TBuffer[4] = 0x01; //FlashMode   Data Buffer[1]
			TBuffer[5] = LEDSetting;  //Data Buffer[2]
			TBuffer[6] = pPeriod;  //Data Buffer[3]
		}
	}
	ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_WRITE_BLUK_FAIL;
	}
	return 1;
}

//------------------------------------------------------------------
//T:|0x6F|0x02|0x00|0x03|LED R/W|
//R:|0x8F|0x01|0x00|0x03|LED ON/OFF|
//------------------------------------------------------------------
int TTK_Communication_API::GetLED_Bulk(unsigned char* LEDSetting)
{
	unsigned char TBuffer[BulkLen] = { 0 };
	unsigned char RBuffer[BulkLen] = { 0 };
	int ret, timeout = 100;

	TBuffer[0] = Bridge_T_System_OP;
	TBuffer[1] = 0x02;
	TBuffer[2] = 0x00;
	TBuffer[3] = 0x03; //LED Status Control
	TBuffer[4] = 0x80; //read

	ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_WRITE_BLUK_FAIL;
	}

	SleepInProgram(5);

	while (timeout--)
	{
		ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
		if (ret > 0)
		{
			if (RBuffer[0x00] == 0x8F && RBuffer[0x03] == 0x03)
			{
				*LEDSetting = RBuffer[0x04];
				break;
			}
		}
	}

	if (timeout == 0)
		return ERRORMSG_TIMEOUT;

	return 1;
}

//------------------------------------------------------------------
//T:|0x6F|0x03|0x00|0x04|Button W|Button ON/OFF|
//------------------------------------------------------------------
int TTK_Communication_API::SetButton_Bulk(unsigned char ButtonSetting)
{
	unsigned char TBuffer[BulkLen] = { 0 };
	int ret;

	TBuffer[0] = Bridge_T_System_OP;
	TBuffer[1] = 0x03;
	TBuffer[2] = 0x00;
	TBuffer[3] = 0x04; //Button Status Control
	TBuffer[4] = 0x00; //write
	TBuffer[5] = ButtonSetting;

	ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_WRITE_BLUK_FAIL;
	}
	return 1;
}

//------------------------------------------------------------------
//T:|0x6F|0x02|0x00|0x04|Button R|
//R:|0x8F|0x01|0x00|0x04|Button Flag|Button78 value|
//------------------------------------------------------------------
int TTK_Communication_API::GetButton_Bulk(unsigned char* ButtonSetting)
{
	unsigned char TBuffer[BulkLen] = { 0 };
	unsigned char RBuffer[BulkLen] = { 0 };
	int ret, timeout = 100;

	TBuffer[0] = Bridge_T_System_OP;
	TBuffer[1] = 0x02;
	TBuffer[2] = 0x00;
	TBuffer[3] = 0x04; //Button Status Control
	TBuffer[4] = 0x80; //read

	ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_WRITE_BLUK_FAIL;
	}
	while (timeout--)
	{
		ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
		if (ret > 0)
		{
			if (RBuffer[0x00] == 0x8F && RBuffer[0x03] == 0x04)
			{
				*ButtonSetting = RBuffer[0x04];        //Button Flag
				*ButtonSetting += (RBuffer[0x05] << 4);  //Button78 value
				break;
			}
		}
	}
	if (timeout == 0)
		return ERRORMSG_TIMEOUT;

	return 1;
}

//----------------------------------------------------------------------------------
//T:|0x6F|0x0A|0x00|0x02|RegWrite|ADDR0|ADDR1|ADDR2|ADDR3|Data0|Data1|Data2|Data3|
//----------------------------------------------------------------------------------
int  TTK_Communication_API::WriteRegTL_Bulk(unsigned int Addr, unsigned int Data)
{
	unsigned char TBuffer[BulkLen] = { 0 };
	int ret;

	TBuffer[0] = Bridge_T_System_OP;
	TBuffer[1] = 0x0A;
	TBuffer[2] = 0x00;
	TBuffer[3] = 0x02; //Register Control
	TBuffer[4] = 0x00; //write
	TBuffer[5] = Addr & 0xFF;
	TBuffer[6] = (Addr >> 8) & 0xFF;
	TBuffer[7] = (Addr >> 16) & 0xFF;
	TBuffer[8] = (Addr >> 24) & 0xFF;
	TBuffer[9] = Data & 0xFF;
	TBuffer[10] = (Data >> 8) & 0xFF;
	TBuffer[11] = (Data >> 16) & 0xFF;
	TBuffer[12] = (Data >> 24) & 0xFF;

	ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_WRITE_BLUK_FAIL;
	}
	return 1;
}

//----------------------------------------------------------------------------------
//T:|0x6F|0x06|0x00|0x02|RegWrite|ADDR0|ADDR1|ADDR2|ADDR3|
//R:|0x8F|0x04|0x00|0x02|Data0|Data1|Data2|Data3|
//----------------------------------------------------------------------------------
int  TTK_Communication_API::ReadRegTL_Bulk(unsigned int Addr, unsigned int* Data)
{
	unsigned char TBuffer[BulkLen] = { 0 };
	unsigned char RBuffer[BulkLen] = { 0 };
	int ret, timeout = 10;

	TBuffer[0] = Bridge_T_System_OP;
	TBuffer[1] = 0x06;
	TBuffer[2] = 0x00;
	TBuffer[3] = 0x02; //Register Control
	TBuffer[4] = 0x80; //Read
	TBuffer[5] = Addr & 0xFF;
	TBuffer[6] = (Addr >> 8) & 0xFF;
	TBuffer[7] = (Addr >> 16) & 0xFF;
	TBuffer[8] = (Addr >> 24) & 0xFF;

	ret = USBComm_WriteToBulkEx(TBuffer, BulkLen);
	if (ret < 0)
	{
		return ERRORMSG_WRITE_BLUK_FAIL;
	}
	SleepInProgram(10);
	while (timeout--)
	{
		ret = USBComm_ReadFromBulkEx(RBuffer, BulkLen);
		if (ret > 0)
		{
			if (RBuffer[0x00] == 0x8F && RBuffer[0x03] == 0x02)
			{
				*Data = RBuffer[0x04];
				*Data += RBuffer[0x05] * 0x100;
				*Data += RBuffer[0x06] * 0x10000;
				*Data += RBuffer[0x07] * 0x1000000;
				break;
			}
		}
	}
	if (timeout == 0)
		return ERRORMSG_TIMEOUT;

	return 1;
}

int TTK_Communication_API::EraseFlashHW(unsigned int Addr, unsigned int Len)
{
	if(m_Chipid == CHIP_ID::A2152)
		return  ST1802EraseFlashHW(Addr, Len);

	if (m_Chipid == CHIP_ID::A8018)
		return EraseFlashHW_A8018(Addr, Len, true);

	int ret, i = 0;
	unsigned int Eindex = 0, curPos, rtLen;
#if IC_Module==IC_Module_A8008 
	unsigned char PBuffer[PageSize512]; // 1024 bytes
#elif  IC_Module==IC_Module_A8010
	unsigned char PBuffer[PageSize1K]; // 1024 bytes
#elif IC_Module == IC_Module_A8015
	unsigned char PBuffer[PageSize512];
#elif (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
	unsigned char PBuffer[PageSize1K * 4]; // 1024 bytes
#elif (IC_Module==IC_Module_A8018)
	unsigned char PBuffer[PageSize1K];
#endif
	unsigned short PAddr = (unsigned short)Addr;
	unsigned char ReadFlag = false;
	unsigned short curPageSize = 0;
	unsigned short validMask = 0;

#if  IC_Module==IC_Module_A8008
	curPageSize = 512;
	validMask = 0x1FF;
#elif  IC_Module==IC_Module_A8010
	curPageSize = 1024;
	validMask = 0x1FF;
#elif IC_Module == IC_Module_A8015
	curPageSize = 512;
	validMask = 0x1FF;
#elif  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
	curPageSize = 4096;
	validMask = 0x1FF;
	if (ST1801FlashWakeUp() == false) {
		if (ST1801ResetTouchLinkPullHight() == false) {
			return false;
		}
		return false;
	}
	if (ST1801_SPI_FLASH_LOCK(FALSE) == false) {
		if (ST1801ResetTouchLinkPullHight() == false) {
			return false;
		}
		return false;
	}
#endif  
	while (Eindex < Len)
	{
		ReadFlag = false;
		if (Eindex == 0)
		{
			//===============read back===========
			if ((PAddr & validMask) || (((Addr + Len) - PAddr) < curPageSize && (Addr + Len) & validMask))
			{
				ret = ExReadFlashPage_Bulk(PAddr & 0xFF00, PBuffer);
				if (ret == false)
				{
					//ShowMessage("EraseFlashHW:ReadFlashPage_Bulk");
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
					if (ST1801ResetTouchLinkPullHight() == false) {
						return false;
					}
					if (ST1801_SPI_FLASH_LOCK(TRUE) == false)   return false;
#endif
					return ERRORMSG_ATOM_READ_FLASH_FAIL;
				}
				ReadFlag = true;
			}

			for (i = PAddr & validMask; (i < curPageSize) && (Eindex < Len); i++, Eindex++)
			{
				PBuffer[i] = 0xFF;
			}
		}
		else {
			//===============read back===========
			if (((Addr + Len) - (PAddr & 0xFF00)) < PageSize && ((Addr + Len) & 0xFF)) {
				ret = ExReadFlashPage_Bulk(PAddr & 0xFF00, PBuffer);
				if (ret == false) {
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
					if (ST1801ResetTouchLinkPullHight() == false) {
						return false;
					}
					if (ST1801_SPI_FLASH_LOCK(TRUE) == false)   return false;
#endif
					return ERRORMSG_ATOM_READ_FLASH_FAIL;
				}
				ReadFlag = true;
			}
			for (i = 0; (i < curPageSize) && (Eindex < Len); i++, Eindex++)
			{
				PBuffer[i] = 0xFF;
			}
		}
		ret = ExEraseFlashPage_Bulk(PAddr & 0xFF00);
		if (ret == false)
		{
			//ShowMessage("EraseFlashHW:EraseFlashPage_Bulk");
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
			if (ST1801_SPI_FLASH_LOCK(TRUE) == false)   return false;
#endif
			return ERRORMSG_ATOM_ERASE_FLASH_FAIL;
		}
		if (ReadFlag == true)
		{
			if (ret == false)
			{
				//ShowMessage("EraseFlashHW:WriteFlashPage_Bulk");
				return ERRORMSG_ATOM_WRITE_FLASH_FAIL;
			}
		}
		PAddr += curPageSize;
	}
#if  (IC_Module==IC_Module_ST1801) || (IC_Module==IC_Module_ST1802)
	if (ST1801_SPI_FLASH_LOCK(TRUE) == false)   return false;
	if (ST1801ResetTouchLinkPullHight() == false) {
		return false;
	}
#endif
	return true;
}