#include <growl++.hpp>
#include <growl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


Growl::Growl(const Growl_Protocol _protocol, const char *const _password, const char *const _application, const char **_notifications, const int _notifications_count)
{
	server = strdup("localhost");

	if (_password)
		password = strdup(_password);
	else
		password = 0;

	protocol = _protocol;
	application = strdup(_application);
	Register(_notifications, _notifications_count);
}

Growl::Growl(const Growl_Protocol _protocol, const char *const _password, const char *const _application, const char **_notifications, const int _notifications_count, const char *const _icon)
{
	server = strdup("localhost");

	if (_password)
		password = strdup(_password);
	else
		password = 0;

	protocol = _protocol;
	application = strdup(_application);
	Register(_notifications, _notifications_count, strdup(_icon));
}

Growl::Growl(const Growl_Protocol _protocol, const char *const _server, const char *const _password, const char *const _application, const char **_notifications, const int _notifications_count )
{
	server = strdup(_server);

	if (_password)
		password = strdup(_password);
	else
		password = 0;

	protocol = _protocol;
	application = strdup(_application);
	Register(_notifications, _notifications_count);
}


void Growl::Register(const char **const notifications, const int notifications_count , const char *const icon )
{
	int res(0);

	if( protocol == GROWL_TCP )
		res = growl_tcp_register( server , application , notifications , notifications_count , password , icon );
	else
		res = growl_udp_register( server , application , notifications , notifications_count , password );

	mConnected = (res==0);
}


Growl::~Growl()
{
	growl_shutdown();

	if(server != NULL)
	{
		free(server);
	}
	if(password != NULL)
	{
		free(password);
	}
	if(application == NULL)
	{
		free(application);
	}
}


void Growl::Notify(const char *const notification, const char *const title, const char* const message)
{
	Growl::Notify(notification, title, message, NULL, NULL);
}


void Growl::Notify(const char *const notification, const char *const title, const char* const message, const char *const url, const char *const icon)
{
	if( !mConnected )
		return;

	if( protocol == GROWL_TCP )
	{
		growl_tcp_notify( server , application , notification , title , message , password , url , icon );
	}
	else
	{
		growl_udp_notify( server , application , notification , title , message , password );
	}
}
