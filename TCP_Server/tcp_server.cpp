#include "tcp_server.hpp"

#if LWIP_TCP

TCPServer::TCPServer(void)
{
	isListenning = false;
	maxConnectionAllowed = 5;
}


err_t TCPServer::init(uint16_t _portNumber)
{
	err_t err;
	portNumber = _portNumber;
  /* create new tcp pcb */
  tcp_server_pcb = tcp_new();

  if (tcp_server_pcb != NULL)
  {
    
    err = tcp_bind(tcp_server_pcb, IP_ADDR_ANY, portNumber);
    
    if (err == ERR_OK)
    {
      /* start tcp listening for pcb */
      tcp_server_pcb = tcp_listen(tcp_server_pcb);
      

      tcp_arg(tcp_server_pcb, this);
      /* initialize LwIP tcp_accept callback function */
      tcp_accept(tcp_server_pcb, TCPServer::accept);
      isListenning = true;
    }
    else 
    {
      /* deallocate the pcb */
      memp_free(MEMP_TCP_PCB, tcp_server_pcb);
			return err;
    }
  }
	return err;
}

err_t TCPServer::accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	TCPServer *self = static_cast<TCPServer*>(arg);
  err_t ret_err;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);
	
	if(err == ERR_OK)
	{
    tcp_arg(newpcb, self);
    
    /* initialize lwip tcp_recv callback function for newpcb  */ 
    tcp_recv(newpcb, TCPServer::recv);
    
    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, TCPServer::error);
		
		//tcp_Server_NewConnection(newpcb);
    if(self->onNewConnection)
    {
    	self->onNewConnection(self,newpcb);
    }
    
    self->clients.push_back(newpcb);
	if(self->clients.size() >= self->getMaxConnectionAllowed())
	{
		self->stopListen();
	}

    ret_err = ERR_OK;
  }
  else
  {
    /*  close tcp connection */
    self->connection_close(newpcb);
    ret_err = ERR_MEM;
  }
  return ret_err;  
}


err_t TCPServer::recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	TCPServer *self = static_cast<TCPServer*>(arg);
  err_t ret_err;
	
  if (p == NULL)
  {
    self->connection_close(tpcb);
    ret_err = ERR_OK;
  }
  else if(err != ERR_OK)
  {
    pbuf_free(p);
    ret_err = err;
  }
  else
	{
	if(self->onReceive)
	{
		self->onReceive(self,tpcb,(uint8_t*)p->payload,p->len);
	}
	tcp_recved(tpcb,p->len);
	tcp_sent(tpcb, NULL);
	pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

void TCPServer::error(void *arg, err_t err)
{
	TCPServer *self = static_cast<TCPServer*>(arg);
	LWIP_UNUSED_ARG(err);
}

err_t TCPServer::connection_close(struct tcp_pcb *tpcb)
{
	err_t err = ERR_OK;
	if(onDisconnect)
	{
	  onDisconnect(this,tpcb);
	}
  
  /* remove all callbacks */
  tcp_arg(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  
  /* close tcp connection */
  err = tcp_close(tpcb);
  clients.remove(tpcb);
  if(!isListenning)
  {
      if(clients.size() < maxConnectionAllowed)
          init(portNumber);
  }
  return err;
}

void TCPServer::stopListen(void)
{
	if(tcp_server_pcb == NULL)
	{
		isListenning = false;
		return;
	}


	tcp_accept(tcp_server_pcb, NULL);
	tcp_close(tcp_server_pcb);

	tcp_server_pcb = nullptr;
	isListenning = false;
}

bool TCPServer::getIsListenning(void)
{
	return isListenning;
}

void TCPServer::setMaxConnectionAllowed(uint8_t _maxConnectionAllowed)
{
	maxConnectionAllowed = _maxConnectionAllowed;
}

uint8_t TCPServer::getMaxConnectionAllowed(void)
{
	return maxConnectionAllowed;
}

void TCPServer::setTCPClientReceiveEvent(TCPClientReceiveEvent cb)
{
	onReceive = cb;
}

void TCPServer::setTCPClientNewConnectionEvent(TCPClientNewConnectionEvent cb)
{
	onNewConnection = cb;
}

void TCPServer::setTCPClientDisconnectEvent(TCPClientDisconnectEvent cb)
{
	onDisconnect = cb;
}

err_t TCPServer::SendData(struct tcp_pcb *tpcb,uint8_t * str , uint16_t _Len)
{
	if(_Len == 0)
	{
		while(str[_Len] != 0)_Len++;
	}
	return tcp_write(tpcb,str,_Len,1);
}

#endif /* LWIP_TCP */
