/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TCP_CLIENT_H__
#define __TCP_CLIENT_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include <stdio.h>
#include <string.h>

class TCPClient;

typedef void (*TCPClientReceiveEvent)(TCPClient *client, uint8_t *buf, uint32_t len);
typedef void (*TCPClientConnectEvent)(TCPClient *client);
typedef void (*TCPClientDisconnectEvent)(TCPClient *client);

class TCPClient
{
public:
	err_t connect(uint16_t _portNumber,uint8_t *_ipAddress);
	err_t SendData(uint8_t * str , uint16_t len);
	void close(void);
	tcp_pcb* getTpcb(void);
	void setTpcb(tcp_pcb *_tpcb);
	bool isConnected(void);

	void setReceiveEvent(TCPClientReceiveEvent cb);
	void setConnectEvent(TCPClientConnectEvent cb);
	void setDisconnectEvent(TCPClientDisconnectEvent cb);

private:
	struct tcp_pcb *tpcb;
	bool connectedState;

	TCPClientReceiveEvent onReceive = nullptr;
	TCPClientConnectEvent onConnect = nullptr;
	TCPClientDisconnectEvent onDisconnect = nullptr;

	static err_t recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
	static err_t connected(void *arg, struct tcp_pcb *tpcb, err_t err);
	static void error(void *arg, err_t err);
};



#endif /* __TCP_CLIENT_H__ */
