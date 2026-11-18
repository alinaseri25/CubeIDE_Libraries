#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "main.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "string.h"
#include <list>

class TCPServer;

typedef void (*TCPClientReceiveEvent)(TCPServer *_server,struct tcp_pcb *_newpcb, uint8_t *_buf, uint32_t _len);
typedef void (*TCPClientNewConnectionEvent)(TCPServer *_server,struct tcp_pcb *_newpcb);
typedef void (*TCPClientDisconnectEvent)(TCPServer *_server,struct tcp_pcb *_newpcb);

class TCPServer
{
public:
	TCPServer(void);
	err_t init(uint16_t _portNumber);
	err_t SendData(struct tcp_pcb *tpcb,uint8_t * str , uint16_t len);
	err_t connection_close(struct tcp_pcb *tpcb);
	void stopListen(void);
	bool getIsListenning(void);
	void setMaxConnectionAllowed(uint8_t _maxConnectionAllowed);
	uint8_t getMaxConnectionAllowed(void);

	void setTCPClientReceiveEvent(TCPClientReceiveEvent cb);
	void setTCPClientNewConnectionEvent(TCPClientNewConnectionEvent cb);
	void setTCPClientDisconnectEvent(TCPClientDisconnectEvent cb);
private:
	uint16_t portNumber;
	struct tcp_pcb *tcp_server_pcb;
	bool isListenning;
	std::list<tcp_pcb*> clients;
	uint8_t maxConnectionAllowed;

	TCPClientReceiveEvent onReceive = nullptr;
	TCPClientNewConnectionEvent onNewConnection = nullptr;
	TCPClientDisconnectEvent onDisconnect = nullptr;

	static err_t accept(void *arg, struct tcp_pcb *newpcb, err_t err);
	static err_t recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
	static void error(void *arg, err_t err);
};


#endif /* __TCP_SERVER */
