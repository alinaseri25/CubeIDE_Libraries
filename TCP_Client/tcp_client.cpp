/* Includes ------------------------------------------------------------------*/
#include <tcp_client.hpp>

#if LWIP_TCP

tcp_pcb* TCPClient::getTpcb(void)
{
	return tpcb;
}

void TCPClient::setTpcb(tcp_pcb *_tpcb)
{
	tpcb = _tpcb;
}

bool TCPClient::isConnected(void)
{
	return connectedState;
}

err_t TCPClient::recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	TCPClient *self = static_cast<TCPClient*>(arg);
	err_t ret_err;

	/* if we receive an empty tcp frame from server => close connection */
	if (p == NULL)
	{
		/* we're done sending, close connection */
		self->close();
		ret_err = ERR_OK;
	}
	/* else : a non empty frame was received from server but for some reason err != ERR_OK */
	else if(err != ERR_OK)
	{
		/* free received pbuf*/
		if (p != NULL)
		{
			pbuf_free(p);
		}
			ret_err = err;
		}
		else
		{
			//tcp_Client_Recieve(tpcb,p->payload,p->len);
			if(self->onReceive)
			{
				self->onReceive(self,(uint8_t *)p->payload,p->len);
			}

			/* Acknowledge data reception */
			tcp_recved(tpcb, p->tot_len);

			pbuf_free(p);
			ret_err = ERR_OK;
		}
	return ret_err;
}

err_t TCPClient::connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	TCPClient *self = static_cast<TCPClient*>(arg);
	if (err == ERR_OK)
	{
		self->setTpcb(tpcb);
		//tcp_arg(tpcb, self);
		//
		//tcp_arg(self->tpcb, self);
		//tpcb = self->tpcb;

		/* initialize LwIP tcp_recv callback function */
		tcp_recv(self->getTpcb(), TCPClient::recv);

		tcp_err(self->getTpcb(), TCPClient::error);

		if(self->onConnect)
		{
			self->onConnect(self);
		}

		self->connectedState = true;
		return ERR_OK;
	}
	else
	{
		/* close connection */
		self->close();
	}
	return err;
}

void TCPClient::error(void *arg, err_t err)
{
	TCPClient *self = static_cast<TCPClient*>(arg);
	if(err == ERR_CONN || err == ERR_ABRT || err == ERR_CLSD || err == ERR_RST)
	{
		self->close();
	}
}

err_t TCPClient::SendData(uint8_t *str,uint16_t len)
{
	err_t ret_err;
	if(tpcb == nullptr)
	{
		ret_err = ERROR;
	}
	else
	{
		ret_err = tcp_write(tpcb,str,len,1);
	}
	if(ret_err != ERR_OK)
	{
		close();
	}
	return ret_err;
}

err_t TCPClient::connect(uint16_t _portNumber,uint8_t* _ipAddress)
{
	err_t ret_err;
	ip_addr_t DestIPaddr;

	/* create new tcp pcb */
	tpcb = tcp_new();

	if (tpcb != NULL)
	{
		IP4_ADDR( &DestIPaddr, _ipAddress[0], _ipAddress[1], _ipAddress[2], _ipAddress[3] );

		tcp_arg(tpcb, this);

		/* connect to destination address/port */
		ret_err = tcp_connect(tpcb,&DestIPaddr,_portNumber,TCPClient::connected);
	}
	else
	{
		memp_free(MEMP_TCP_PCB, tpcb);
		ret_err = ERR_MEM;
	}
	return ret_err;
}

void TCPClient::close(void)
{
	/* remove callbacks */
	tcp_arg(tpcb, nullptr);
	tcp_recv(tpcb, nullptr);
	tcp_err(tpcb, nullptr);
	/* close tcp connection */
	tcp_close(tpcb);
	tpcb = nullptr;
	connectedState = false;
	if(onDisconnect)
	{
		onDisconnect(this);
	}
}

void TCPClient::setReceiveEvent(TCPClientReceiveEvent cb)
{
    onReceive = cb;
}

void TCPClient::setConnectEvent(TCPClientConnectEvent cb)
{
    onConnect = cb;
}

void TCPClient::setDisconnectEvent(TCPClientDisconnectEvent cb)
{
	onDisconnect = cb;
}

#endif /* LWIP_TCP */

