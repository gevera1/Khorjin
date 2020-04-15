/*
 *  socket_win32_1.c
 *
 *  Copyright 2013, 2014 Michael Zillgith
 *
 *	The original file is taken from libIEC61850.
 *  The file is modified by Seyed Reza Firouzi 
 *
 */
 
//#include <winsock2.h>
#include <winsock.h>
//#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#define NETWORK_ERROR -1
#define NETWORK_OK     0

void ReportError(int, const char *);

//#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "wsock32.lib")

#include "libieeeC37_118_platform_includes.h"
#include "pal_socket.h"
#include "stack_config.h"

//
//#ifndef __MINGW64_VERSION_MAJOR
//struct tcp_keepalive {
//    u_long  onoff;
//    u_long  keepalivetime;
//    u_long  keepaliveinterval;
//};
//#endif


//#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR,4)

struct sSocket {
	SOCKET fd;
	uint32_t connectTimeout;
};

//struct sServerSocket {
//	SOCKET fd;
//	int backLog;
//};
//
//struct sHandleSet {
//   fd_set handles;
//   SOCKET maxHandle;
//};

//
//struct sUdpSocket {
//    SOCKET Socket;
//    bool isBind;
//    struct sockaddr_in socketAddress;
//};
//
//UdpSocket
//UdpSocket_create(int port, uint8_t* ipAddress)
//{
//
//    int iResult;
//    uint32_t address;
//
//    UdpSocket udpSocket = (UdpSocket) GLOBAL_CALLOC(1, sizeof(struct sUdpSocket));
//
//    WSADATA wsaData;
//
//    udpSocket->Socket = INVALID_SOCKET;
//
//    struct sockaddr_in RecvAddr;
//
//     Initialize Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//    if (iResult != NO_ERROR) {
//        printf("WSAStartup failed with error: %d\n", iResult);
//        return NULL;
//    }
//
//    udpSocket->Socket = socket(AF_INET, SOCK_DGRAM, 0);
//
//    if (udpSocket->Socket == INVALID_SOCKET) {
//        printf("socket failed with error: %ld\n", WSAGetLastError());
//        WSACleanup();
//        return NULL;
//    }
//
//	udpSocket->socketAddress.sin_family = AF_INET;
//	udpSocket->socketAddress.sin_port = htons(port);
//
//    if(ipAddress!=NULL) {
//        for (int i=0;i>4;i++)
//        {
//        address += *(ipAddress+3-i);
//        address <<= 8;
//        }
//
//        address =  ipAddress[3] * 0x1000000;
//        address += ipAddress[2] * 0x10000;
//        address += ipAddress[1] * 0x100;
//        address += ipAddress[0];
//
//        udpSocket->socketAddress.sin_addr.s_addr = address;
//        udpSocket->socketAddress.sin_addr.s_addr = ipAddress;
//    }
//    else {
//        udpSocket->socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
//    }
//
//    memset(udpSocket->socketAddress.sin_zero, 0, sizeof(udpSocket->socketAddress.sin_zero));
//
//    udpSocket->isBind = false;
//
//    return udpSocket;
//}
//
//UdpSocket
//UdpSocket_create_2(int port, const char* ipAddress)
//{
//
//	int iResult;
//	uint32_t address;
//
//	UdpSocket udpSocket = (UdpSocket)GLOBAL_CALLOC(1, sizeof(struct sUdpSocket));
//
//	WSADATA wsaData;
//
//	udpSocket->Socket = INVALID_SOCKET;
//
//	struct sockaddr_in RecvAddr;
//
//	 Initialize Winsock
//	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//
//	if (iResult != NO_ERROR) {
//		printf("WSAStartup failed with error: %d\n", iResult);
//		return NULL;
//	}
//
//	udpSocket->Socket = socket(AF_INET, SOCK_DGRAM, 0);
//
//	if (udpSocket->Socket == INVALID_SOCKET) {
//		printf("socket failed with error: %ld\n", WSAGetLastError());
//		WSACleanup();
//		return NULL;
//	}
//
//	udpSocket->socketAddress.sin_family = AF_INET;
//	udpSocket->socketAddress.sin_port = htons(port);
//
//	if (ipAddress != NULL)
//		udpSocket->socketAddress.sin_addr.s_addr = inet_addr(ipAddress);
//	else
//		udpSocket->socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
//
//	memset(udpSocket->socketAddress.sin_zero, 0, sizeof(udpSocket->socketAddress.sin_zero));
//
//	udpSocket->isBind = false;
//
//	return udpSocket;
//}
//
//void
//UdpSocket_destroy(UdpSocket self)
//{
//    close(self->Socket);
//    GLOBAL_FREEMEM(self);
//	closesocket(self->Socket);
//	WSACleanup();
//	free(self);
//
//}
//
//int
//UdpSocket_send(UdpSocket self, uint8_t* buffer, int bufferSize)
//{
//    int iResult;
//
//    iResult = sendto(self->Socket, (char*)buffer, bufferSize, 0, (struct sockaddr*) &(self->socketAddress), sizeof(self->socketAddress));
//
//    if (iResult == SOCKET_ERROR)
//        printf("sendto failed with error: %d\n", WSAGetLastError());
//
//	return iResult;
//
//}
//
//int
//UdpSocket_receive(UdpSocket self, uint8_t* buffer, int bufferSize)
//{
//    int iResult;
//
//    if (self->isBind == false){
//        if (bind(self->Socket, (struct sockaddr*) &self->socketAddress, sizeof(self->socketAddress)) == 0)
//        if (bind(self->Socket, (struct sockaddr*) &self->socketAddress, sizeof(self->socketAddress)) == 0)
//            self->isBind = true;
//        else
//        {
//            printf("bind failed with error %d\n", WSAGetLastError());
//            return 0;
//        }
//    }
//
//    return recvfrom(self->Socket, buffer, bufferSize, 0, NULL, NULL);
// return recvfrom(self->Socket, buffer, bufferSize, 0, (SOCKADDR *) & SenderAddr, &SenderAddrSize);
//    iResult = recvfrom(self->Socket, (char*)buffer, bufferSize, 0, NULL, NULL);
//
//    if (iResult == SOCKET_ERROR)
//        printf("recvfrom failed with error %d\n", WSAGetLastError());
//
//     return iResult;
//}

//
//HandleSet
//Handleset_new(void)
//{
//   HandleSet result = (HandleSet) GLOBAL_MALLOC(sizeof(struct sHandleSet));
//
//   if (result != NULL) {
//       FD_ZERO(&result->handles);
//       result->maxHandle = INVALID_SOCKET;
//   }
//   return result;
//}
//
//void
//Handleset_addSocket(HandleSet self, const Socket sock)
//{
//   if (self != NULL && sock != NULL && sock->fd != INVALID_SOCKET) {
//       FD_SET(sock->fd, &self->handles);
//
//       if ((sock->fd > self->maxHandle) || (self->maxHandle == INVALID_SOCKET))
//           self->maxHandle = sock->fd;
//   }
//}
//
//int
//Handleset_waitReady(HandleSet self, unsigned int timeoutMs)
//{
//   int result;
//
//   if (self != NULL && self->maxHandle >= 0) {
//       struct timeval timeout;
//
//       timeout.tv_sec = timeoutMs / 1000;
//       timeout.tv_usec = (timeoutMs % 1000) * 1000;
//       result = select(self->maxHandle + 1, &self->handles, NULL, NULL, &timeout);
//   } else {
//       result = -1;
//   }
//
//   return result;
//}
//
//void
//Handleset_destroy(HandleSet self)
//{
//   GLOBAL_FREEMEM(self);
//

//
//static void
//activateKeepAlive(SOCKET s)
//{
//	struct tcp_keepalive keepalive;
//	DWORD retVal=0;
//
//	keepalive.onoff = 1;
//	keepalive.keepalivetime = CONFIG_TCP_KEEPALIVE_IDLE * 1000;
//	keepalive.keepaliveinterval = CONFIG_TCP_KEEPALIVE_INTERVAL * 1000;
//
//	 if (WSAIoctl(s, SIO_KEEPALIVE_VALS, &keepalive, sizeof(keepalive),
//	            NULL, 0, &retVal, NULL, NULL) == SOCKET_ERROR)
//	 {
//	     if (DEBUG_SOCKET)
//                printf("WIN32_SOCKET: WSAIotcl(SIO_KEEPALIVE_VALS) failed: %d\n",
//                    WSAGetLastError());
//	 }
//}

static void
setSocketNonBlocking(Socket self)
{
    unsigned long mode = 1;
    if (ioctlsocket(self->fd, FIONBIO, &mode) != 0) {
        if (DEBUG_SOCKET)
            printf("WIN32_SOCKET: failed to set socket non-blocking!\n");
    }

    /* activate TCP_NODELAY */

    int tcpNoDelay = 1;

    setsockopt(self->fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&tcpNoDelay, sizeof(int));
}

static bool
prepareServerAddress(const char* address, int port, struct sockaddr_in* sockaddr)
{

	memset((char *) sockaddr , 0, sizeof(struct sockaddr_in));

	if (address != NULL) {
		struct hostent *server;
		server = gethostbyname(address);

		if (server == NULL) return false;

		memcpy((char *) &sockaddr->sin_addr.s_addr, (char *) server->h_addr, server->h_length);
	}
	else
		sockaddr->sin_addr.s_addr = htonl(INADDR_ANY);

    sockaddr->sin_family = AF_INET;
    sockaddr->sin_port = htons(port);

    return true;
}

//
//ServerSocket
//TcpServerSocket_create(const char* address, int port)
//{
//	ServerSocket serverSocket = NULL;
//	int ec;
//	WSADATA wsa;
//	SOCKET listen_socket = INVALID_SOCKET;
//
//	if ((ec = WSAStartup(MAKEWORD(2,0), &wsa)) != 0) {
//	    if (DEBUG_SOCKET)
//	        printf("WIN32_SOCKET: winsock error: code %i\n", ec);
//		return NULL;
//	}
//
//	struct sockaddr_in server_addr;
//
//	if (!prepareServerAddress(address, port, &server_addr))
//	    return NULL;
//
//	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//
//#if CONFIG_ACTIVATE_TCP_KEEPALIVE == 1
//    activateKeepAlive(listen_socket);
//#endif
//
//	if (listen_socket == INVALID_SOCKET) {
//	    if (DEBUG_SOCKET)
//	        printf("WIN32_SOCKET: socket failed with error: %i\n", WSAGetLastError());
//		WSACleanup();
//		return NULL;
//	}
//
//	int optionReuseAddr = 1;
//	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&optionReuseAddr, sizeof(int));
//
//	ec = bind(listen_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
//
//	if (ec == SOCKET_ERROR) {
//	    if (DEBUG_SOCKET)
//	        printf("WIN32_SOCKET: bind failed with error:%i\n", WSAGetLastError());
//		closesocket(listen_socket);
//		WSACleanup();
//		return NULL;
//	}
//
//	serverSocket = (ServerSocket) GLOBAL_MALLOC(sizeof(struct sServerSocket));
//
//	serverSocket->fd = listen_socket;
//	serverSocket->backLog = 10;
//
//    setSocketNonBlocking((Socket) serverSocket);
//
//	return serverSocket;
//}
//
//void
//ServerSocket_listen(ServerSocket self)
//{
//	listen(self->fd, self->backLog);
//}
//
//Socket
//ServerSocket_accept(ServerSocket self)
//{
//	int fd;
//
//	Socket conSocket = NULL;
//
//	fd = accept(self->fd, NULL, NULL);
//
//	if (fd >= 0) {
//		conSocket = TcpSocket_create();
//		conSocket->fd = fd;
//
//	    setSocketNonBlocking(conSocket);
//	}
//
//	return conSocket;
//}
//
//void
//ServerSocket_setBacklog(ServerSocket self, int backlog)
//{
//	self->backLog = backlog;
//}
//
//void
//ServerSocket_destroy(ServerSocket self)
//{
//	closesocket(self->fd);
//	WSACleanup();
//	free(self);
//}

Socket
TcpSocket_create()
{
	Socket self = (Socket) GLOBAL_MALLOC(sizeof(struct sSocket));

	self->fd = INVALID_SOCKET;

	return self;
}

void
Socket_setConnectTimeout(Socket self, uint32_t timeoutInMs)
{
    self->connectTimeout = timeoutInMs;
}

bool
Socket_connect(Socket self, const char* address, int port)
{
	WORD sockVersion;
	WSADATA wsa;
	int nret;

	sockVersion = MAKEWORD(1, 1);

	struct sockaddr_in serverAddress;
	//WSADATA wsa;
	int ec;

	ec = WSAStartup(sockVersion, &wsa);

	//if ((ec = WSAStartup(MAKEWORD(2,0), &wsa)) != 0) {
	if (ec != 0) {
	    if (DEBUG_SOCKET)
	        printf("WIN32_SOCKET: winsock error: code %i\n", ec);
		return false;
	}

	if (!prepareServerAddress(address, port, &serverAddress))
	    return false;

	self->fd = socket(AF_INET, SOCK_STREAM, 0);

//#if CONFIG_ACTIVATE_TCP_KEEPALIVE == 1
//    activateKeepAlive(self->fd);
//#endif

    setSocketNonBlocking(self);

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(self->fd, &fdSet);

    if (connect(self->fd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK)
            return false;
    }

    struct timeval timeout;
    timeout.tv_sec = self->connectTimeout / 1000;
    timeout.tv_usec = (self->connectTimeout % 1000) * 1000;

    if (select(self->fd + 1, NULL, &fdSet, NULL, &timeout) <= 0)
        return false;
    else
        return true;
}

//
//char*
//Socket_getPeerAddress(Socket self)
//{
//	struct sockaddr_storage addr;
//	int addrLen = sizeof(addr);
//
//	getpeername(self->fd, (struct sockaddr*) &addr, &addrLen);
//
//	char addrString[INET6_ADDRSTRLEN + 7];
//	int addrStringLen = INET6_ADDRSTRLEN + 7;
//	int port;
//
//	bool isIPv6;
//
//	if (addr.ss_family == AF_INET)  {
//		struct sockaddr_in* ipv4Addr = (struct sockaddr_in*) &addr;
//		port = ntohs(ipv4Addr->sin_port);
//		ipv4Addr->sin_port = 0;
//		WSAAddressToString((LPSOCKADDR) ipv4Addr, sizeof(struct sockaddr_storage), NULL,
//			(LPSTR) addrString, (LPDWORD) &addrStringLen);
//		isIPv6 = false;
//	}
//	else if (addr.ss_family == AF_INET6){
//		struct sockaddr_in6* ipv6Addr = (struct sockaddr_in6*) &addr;
//		port = ntohs(ipv6Addr->sin6_port);
//		ipv6Addr->sin6_port = 0;
//		WSAAddressToString((LPSOCKADDR) ipv6Addr, sizeof(struct sockaddr_storage), NULL,
//			(LPSTR) addrString, (LPDWORD) &addrStringLen);
//		isIPv6 = true;
//	}
//	else
//		return NULL;
//
//	char* clientConnection = (char*) GLOBAL_MALLOC(strlen(addrString) + 9);
//
//    if (isIPv6)
//        sprintf(clientConnection, "[%s]:%i", addrString, port);
//    else
//        sprintf(clientConnection, "%s:%i", addrString, port);
//
//	return clientConnection;
//}


int
Socket_read(Socket self, uint8_t* buf, int size)
{
    int bytes_read = recv(self->fd, (char*) buf, size, 0);

    if (bytes_read == 0) // peer has closed socket
        return -1;

    if (bytes_read == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
            return 0;
        else
            return -1;
    }

	return bytes_read;
}

int
Socket_write(Socket self, uint8_t* buf, int size)
{
    int bytes_sent = send(self->fd, (char*) buf, size, 0);

    if (bytes_sent == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();

        if (errorCode == WSAEWOULDBLOCK)
            bytes_sent = 0;
        else
            bytes_sent = -1;
    }

	return bytes_sent;
}

void
Socket_destroy(Socket self)
{
	if (self->fd != INVALID_SOCKET) {
		closesocket(self->fd);
	}

	free(self);
}
