#include "HFST_WIFI_SDK.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <exception>

static SOCKET g_Socket = INVALID_SOCKET;

//constexpr unsigned int TCP_MTU = 1500;

/*
* @brief    Connect to WIFI server
*
* @param    ip      [in] server ip address
* @param    port    [in] server port
* 
* @throw    throw exception when error occur
*/
void HFST_WIFI_SDK_Connect(const char* ip, unsigned int port)
{
    WSADATA data;
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
    {
        throw std::runtime_error("Init Winsock failed!");
    }

    g_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (g_Socket == INVALID_SOCKET)
    {
        throw std::runtime_error("Create client socket failed!");
    }

    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    if (connect(g_Socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int ret = WSAGetLastError();
        if (ret == WSAECONNREFUSED)
        {
            throw std::runtime_error("Server refused connect request!");
        }
        else
        {
            throw std::runtime_error("Connect to server failed!");
        }
    }
}

/*
* @brief    DisConnect from WIFI server
*
* @throw    throw exception when error occur
*/
void HFST_WIFI_SDK_DisConnect()
{
    if (closesocket(g_Socket) == SOCKET_ERROR)
    {
        throw std::runtime_error("Close client socket failed!");
    }

    if (WSACleanup())
    {
        throw std::runtime_error("Release Winsock2 failed!");
    }
}


/*
* @brief    Recv from WIFI server
*
* @param    buf     [out] output data
* @param    len    [in] recv length
*
* @throw    throw exception when error occur
*/
unsigned int HFST_WIFI_SDK_Recv(unsigned char* buf, unsigned int len)
{
    if (!buf) throw std::invalid_argument("Input buffer is nullptr!");


    int rt = recv( g_Socket, (char*)buf, (int)len, 0 );

    if ( rt == SOCKET_ERROR )
    {
        throw std::runtime_error("Recv from server failed!");
    }

    return rt;
}


/*
* @brief    Send to WIFI server
*
* @param    buf    [in] input data
* @param    len    [in] send length
*
* @throw    throw exception when error occur
*/
void HFST_WIFI_SDK_Send(const char* buf, unsigned int len)
{
    if (!buf) throw std::invalid_argument("Input buffer is nullptr!");

    if (SOCKET_ERROR == send(g_Socket, buf, (int)len, 0))
    {
        throw std::runtime_error("Send to server failed!");
    }
}
