#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<string.h>
#include<time.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<fcntl.h>

#define BUFSIZE 8192

/*	----- file I/O -----	*/
void func_IO( int fd_in, int fd_out )
{
	int ret;
	char* buffer = ( char* )malloc( sizeof( char )*BUFSIZE );

	while( ( ret = read( fd_in, buffer, BUFSIZE ) ) > 0 )
	{
		printf("%s\n", buffer );
		write( fd_out, buffer, ret );
	}

	free( buffer );
	return ;
}

/*	----- get file name -----	*/
char* filename( char* buffer )
{
	char* name = ( char* )malloc( sizeof( char )*1024 );
	
	char* ptr = &buffer[ 0 ];
	
	int count = 0;
	/*	----- remove the head -----	*/
	while( *ptr != '\0' && *ptr != '\n' && *ptr != EOF && *ptr != '\r' )
	{
		if( *ptr == 47 )
		{
			ptr++;
			break;
		}
		ptr++;
	}

	
	/*	----- get file name -----	*/
	while( *ptr != '\0' && *ptr != '\n' && *ptr != EOF && *ptr != '\r' )
	{
		if( *ptr == 32 )
		{
			break;
		}
		else
		{
			name[ count ] = *ptr;
			count++;	
		}
		ptr++;
	}

	if( count == 0 )
	{
		strcpy( name, "1104.html" );
	}
	else
		return name;
}

void type_handler( char* name, char* buffer )
{
	char* type = ( char* )malloc( sizeof( char ) * 8 );
	int count = 0;
	char* ptr = &name[ 0 ];

	while( *ptr != '\n' && *ptr != '\0' && *ptr != EOF )
	{
		if( *ptr == '.' )
		{
			ptr++;
			break;
		}
		ptr++;
	}
	
	while( *ptr != '\0' && *ptr != EOF )
	{
		type[ count ] = *ptr;
		ptr++;
		count++;
	}
	type[ count ] = '\0';
	
	if( strcmp( type, "html" ) == 0 )
	{
		sprintf( buffer, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
	}
	else if( strcmp( type, "jpg" ) == 0 )
	{
		sprintf( buffer, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");
	}

	free( type );
	return ;
}
void show( char* ptr )
{
	printf("show: ");
	for( int i = 0; i < 40; i++ )
	{
		printf("%c", *ptr );
		ptr++;
	}
	printf("\n\n");
	return ;
}
void func_post( int connfd, char* buffer )
{
	int cntlen = 0;
	int count = 0;
	int offset = 85;
	char* line = ( char* )malloc( sizeof( char )*( BUFSIZE + 1 ) );
	char* name = ( char* )malloc( sizeof( char )*( BUFSIZE + 1 ) );


	printf("buffer: \n\n%s\n\n", buffer );
	memset( name, '\0', BUFSIZE );
	char* ptr = strstr( buffer, "Content-Length: ");

	ptr = ptr + strlen("Content-Length: ");
	cntlen = atoi( ptr );

	ptr = strstr( buffer, "filename");

	if( ptr == NULL )
	{
		printf("func_post, strstr( buffer, 'filename'), error: NULL \n");
		return ;
	}

	ptr = ptr + strlen("filename= ");

	while( *ptr != '\"' )
	{
		name[ count ] = *ptr;
		count++;
		ptr++;
	}

	ptr = ptr + 5;
	ptr = strstr( ptr, "\r\n" );// content starts...
	offset = offset + strlen( name );
	
	printf("name: %s, offset: %d, cntlen: %d\n", name, offset, cntlen );
	int fd_out = open( name, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU );
	if( fd_out < 0 )
	{
		perror("func_post, open()-fd_out, error: ");
		return ;
	}

	int remain = cntlen - offset;
	char* end;

	int time = 0;
	char* shortline = ( char* )malloc( sizeof( char )*32 );

	/*	first round ( with end )	*/
	if( ( end = strstr( ptr, "------WebKitFormBoundary" ) ) != NULL )
	{
		printf("enter case 1 ------------------------------------------------------------------------------------------\n");
		count = 0;
		memset( line, '\0', BUFSIZE );
		while( ptr != end )
		{
			line[ count ] = *ptr;
			count++;
			ptr++;
		}
		memset( shortline, '\0', 32 );
		sprintf( shortline, "{ write time: %d }", time );
		time++;
//		write( fd_out, shortline, 32 );
		write( fd_out, line, count );
		printf("line: \n\n%s\n\n", line );
		remain = 0;
	}
	else	/*	first round ( without end )	*/
	{
		printf("enter case 2 -----------------------------------------------------------------------------------------\n");
		memset( line, '\0', BUFSIZE );
		while( count < BUFSIZE && *ptr > 0 )
		{
			line[ count ] = *ptr;
			count++;
			ptr++;
		}

		memset( shortline, '\0', 32 );
		sprintf( shortline, "{ write time: %d }", time );
		time++;
//		write( fd_out, shortline, 32 );
		printf("shortline: %s\n\n", shortline );
		write( fd_out, line, count );
		printf("line:\n\n%s\n\n", line );
		remain = remain - count;
	}

	int tmp = 0;

	/*	rounds ( without end )	*/
	printf("before big one.--------------------------------------------------------------------------------------------------\n");
	while( remain > BUFSIZE )
	{
		printf("-----------------------------------\n\n\t\tinside while\n\n-----------------------------------\n\n");
		count = 0;
		memset( buffer, '\0', BUFSIZE );
		read( connfd, buffer, BUFSIZE );

		if( ( end = strstr( buffer, "------WebKitFormBoundary" ) ) != NULL )
		{
			printf("end found!!\n");
			show( end );
			break;
		}

		memset( shortline, '\0', 32 );
		sprintf( shortline, "{ write time: %d )", time );
		time++;
//		write( fd_out, shortline, 32 );
		printf("shortline: %s\n\n", shortline );
		write( fd_out, buffer, BUFSIZE );
		printf("buffer:\n\n%s\n\n", buffer );

		remain = remain - BUFSIZE;
		
		printf("remain: %d\n", remain );

	}
	printf("remain: %d\n", remain );
	printf("after big one.--------------------------------------------------------------------------------------------------\n");
	
	ptr = buffer;

	printf("last buffer: ==============================================================\n%s\n", buffer );
	printf("last buffer end ===========================================================\n\n");
	/*	find end	*/
	
	if( remain > 0 )
	{
		end = strstr( ptr, "------" );

		if( end == NULL )
		{
			printf("end is NULL\n");
		}
		else
		{
			printf("end exist\n");
		}

		show( ptr );
		show( end );

		*end = '\0';
		
		show( end );

		printf("enter last one\n");

		printf("buffer: %s\n", buffer );
		
		memset( shortline, '\0', 32 );
		sprintf( shortline, "{ write time: %d }", time );
		time++;
//		write( fd_out, shortline, 32 );
		printf("shortline: %s\n\n", shortline );
		write( fd_out, buffer, strlen( buffer ) );
		printf("buffer: \n\n%s\n\n", buffer );

		remain = 0;
	}

	count = 0;
	printf("end func_post----------------------------------------------------------------------------------------------------\n");

	close( fd_out );
	free( line );
	free( name );
	return ;
}


/*	----- method: GET  ----- 	*/
void get_handler( int connfd, char* buffer )
{
	char* name = filename( buffer );


	type_handler( name , buffer );
	write( connfd, buffer, strlen( buffer ) );
	
	int fd_in = open( name, O_RDONLY );

	func_IO( fd_in, connfd );

	close( fd_in );
	return ;
}

/*	----- method: POST -----	*/
void post_handler( int connfd, char* buffer )
{
//	printf("%s\n", buffer );


	func_post( connfd, buffer );
	/*
	
	char* index = ( char* )malloc( sizeof( char )*1024 );

	sprintf( index, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" );
	write( connfd, index, strlen( index ) );

	int fd_index = open( "1104.html", O_RDONLY );
	func_IO( fd_index, connfd );


	close( fd_index );
	free( index );
	*/
	return ;
}

void handler( int connfd )
{
//	printf("\n\n----- handler starts -----\n\n" );

	int ret;
	
	char* buffer = ( char* )malloc( sizeof( char )*( BUFSIZE + 1 ) );
	
	memset( buffer, '\0', BUFSIZE + 1 );

	ret = read( connfd, buffer, BUFSIZE );
	printf("handler, ret: %d\n", ret );

	printf("\n%s\n\n", buffer );

	if( ret <= 0 )
	{
		perror("handler, read(), error: \n");
		exit( 3 );
	}
	


	/*	----- get method -----	*/
	if( strncmp( buffer, "GET ", 4 ) == 0 )
	{
		get_handler( connfd, buffer );
	}
	else if( strncmp( buffer, "POST ", 5 ) == 0 )
	{
		post_handler( connfd, buffer );
	}
		

	free( buffer );
	return ;
}


int main( int argc, char** argv )
{
	int ret;

	int len_inet;
	struct sockaddr_in addr_serv, addr_clnt;
	pid_t pid;

	if( chdir("/mnt/d/internet") == -1 )
	{
		perror("main, chdir(), error: \n");
		return 0;
	}


	/*	consturct listening socket	*/
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );

	if( sockfd < 0)
	{
		perror("main, socket(), error: \n");
		return 0;
	}
	else
	{
		printf("socket fd: %d\n", sockfd );
	}

	addr_serv.sin_family = AF_INET;
	//addr_serv.sin_addr.s_addr = htonl( INADDR_ANY );
	addr_serv.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	addr_serv.sin_port = htons( 80 );


	/*	connect socket	*/
	ret = bind( sockfd, ( struct sockaddr* )&addr_serv, sizeof( addr_serv ) );
	if( ret < 0 )
	{
		
		perror("main, bind(), error: \n");
		return 0;
	}
	printf("binding finish...\n");

	/*	maximum number of listening clients:	*/
	ret = listen( sockfd, 10 );
	if( ret < 0 )
	{
		perror("main, listen(), error: \n");
		return 0;
	}


	int connfd;
	char* line = ( char* )malloc( sizeof( char ) * 1024 );
	while( 1 )
	{
		len_inet = sizeof( addr_clnt );
		
		/*	accept client request, construct connected socket	*/
		connfd = accept( sockfd, ( struct sockaddr* )&addr_clnt, &len_inet );
		/*	construct child process	*/
		pid = fork();
		/*	parent process	*/
		if( pid > 0 )
		{
//			printf("child process %d running...\n", pid );
			close( connfd );
			continue;
		}

		/*	child process	*/
		handler( connfd );

		close( sockfd );
		exit( 0 );
	}

	free( line );
	
	return 0;
}
