#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "tcp.h"

int growl_tcp_parse_hostname( const char *const server , int default_port , struct sockaddr_in *const sockaddr  );

#ifdef _WIN32
#define GROWL_LOG_FUNCENTER(  ) { growl_do_log( "tcp.c: " ## __FUNCTION__, 3 ); }
#define GROWL_LOG( Message ) { growl_do_log( "tcp.c: " ## __FUNCTION__ ## ": " ## Message , 3 ); }
#else
#define GROWL_LOG_FUNCENTER( ) /* nothing */
#define GROWL_LOG( Message ) /* nothing */
#endif

void growl_tcp_write_raw( int sock, const unsigned char * data, const int data_length )
{
	GROWL_LOG_FUNCENTER(  )
	send(sock, data, data_length, 0);
}

void growl_tcp_write( int sock , const char *const format , ... ) 
{
	int length;
	char *output;
	char *stop;

	va_list ap;

	GROWL_LOG_FUNCENTER(  )
	va_start( ap , format );
	length = vsnprintf( NULL , 0 , format , ap );
	va_end(ap);

	va_start(ap,format);
	output = (char*)malloc(length+1);
	if (!output) {
		va_end(ap);
		return;
	}
	vsnprintf( output , length+1 , format , ap );
	va_end(ap);

	while ((stop = strstr(output, "\r\n"))) strcpy(stop, stop + 1);

	send( sock , output , length , 0 );
	send( sock , "\r\n" , 2 , 0 );

	if( getenv("GNTP_DEBUG") != NULL )
	{
		printf( "<%s\n" , output );
	}
	free(output);
}

char *growl_tcp_read(int sock) {
	const int growsize = 80;
	char c = 0;
	char* line = (char*) malloc(growsize);
	GROWL_LOG_FUNCENTER(  )
	if (line) {
		int len = growsize, pos = 0;
		char* newline;
		while (line) {
			if (recv(sock, &c, 1, 0) <= 0) break;
			if (c == '\r') continue;
			if (c == '\n') break;
			line[pos++] = c;
			if (pos >= len) {
				len += growsize;
				newline = (char*) realloc(line, len);
				if (!newline) {
					free(line);
					return NULL;
				}
				line = newline;
			}
		}
		line[pos] = 0;
	}
	
	if( getenv("GNTP_DEBUG") != NULL )
	{
		printf( ">%s\n" , line );
	}

	return line;
}

int growl_tcp_open(const char* server) {
	int sock = -1;
#ifdef _WIN32
	char on;
#else
	int on;
#endif
	struct sockaddr_in serv_addr;

	GROWL_LOG_FUNCENTER(  )
	if( growl_tcp_parse_hostname( server , 23053 , &serv_addr ) == -1 )
	{
		return -1;
	}

    GROWL_LOG( "creating socket" )
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("create socket");
		return -1;
	}

    GROWL_LOG( "connect" )
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("connect");
		return -1;
	}

	on = 1;
    GROWL_LOG( "setsockopt" )
	if (setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) == -1) {
		perror("setsockopt");
		return -1;
	}

	return sock;
}

void growl_tcp_close(int sock) {
#ifdef _WIN32
	if (sock > 0) closesocket(sock);
#else
	if (sock > 0) close(sock);
#endif
}

int growl_tcp_parse_hostname( const char *const server , int default_port , struct sockaddr_in *const sockaddr )
{
	char *hostname = strdup(server);
	char *port = strchr( hostname, ':' );
	struct hostent* host_ent;
	GROWL_LOG_FUNCENTER(  )
	if( port != NULL )
	{
		*port = '\0';
		port++;
		default_port = atoi(port);
	}
	
	host_ent = gethostbyname(hostname);
	if( host_ent == NULL )
	{
		perror("gethostbyname");
		free(hostname);
		return -1;
	}
	
	memset( sockaddr , 0 , sizeof(sockaddr) );
	sockaddr->sin_family = AF_INET;
	memcpy( &sockaddr->sin_addr , host_ent->h_addr , host_ent->h_length );
	sockaddr->sin_port = htons(default_port);
	 
	free(hostname);
	return 0;
}

int growl_tcp_datagram( const char *server , const unsigned char *data , const int data_length )
{
	struct sockaddr_in serv_addr;
	int sock = 0;

	GROWL_LOG_FUNCENTER(  )
	if( growl_tcp_parse_hostname( server , 9887 , &serv_addr ) == -1 )
	{
		return -1;
	}
	
    GROWL_LOG( "socket" )
	sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( sock < 0 )
	{
		return -1;
	}
	
    GROWL_LOG( "sendto" )
	if( sendto(sock, (char*)data , data_length , 0 , (struct sockaddr*)&serv_addr , sizeof(serv_addr) ) > 0 )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
