
#ifndef __BULKDLL_H__
#define __BULKDLL_H__

int USBComm_InitEx(unsigned int TimeOut, unsigned int nDeviceIndex = 1);
int USBComm_FinishEx();

int USBComm_CheckBulkInEx();
int USBComm_CheckBulkOutEx();

int USBComm_ReadFromBulkEx(unsigned char *Buffer, unsigned int BufferSize);
int USBComm_WriteToBulkEx(unsigned char *Buffer, unsigned int BufferSize);

#endif // __BULKDLL_H__