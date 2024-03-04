#ifndef HFST_WIFI_SDK_H_
#define HFST_WIFI_SDK_H_

void _declspec(dllexport) HFST_WIFI_SDK_Connect(const char* ip, unsigned int port);
void _declspec(dllexport) HFST_WIFI_SDK_DisConnect();
unsigned int _declspec(dllexport) HFST_WIFI_SDK_Recv(unsigned char* buf, unsigned int len);
void _declspec(dllexport) HFST_WIFI_SDK_Send(const char* buf, unsigned int len);

#endif //HFST_WIFI_SDK_H_
