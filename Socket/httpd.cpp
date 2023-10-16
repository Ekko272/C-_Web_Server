#include <iostream>
#include <WinSock2.h>
#pragma comment(lib, "WS2_32.lib")

#define PRINTF(str) printf("[%s - %d]"#str"=%s", __func__, __LINE__, str);
using namespace std;
//te
void error_die(const char* str) {
	perror(str);
	exit(1);
}

int startUp(unsigned short* port) {
	// 1. Initalize
	WSADATA data;
	int result =  WSAStartup(MAKEWORD(1, 1), &data);
	if (result) { error_die("WSAStartup"); }

	int server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (server_socket == -1) {
		error_die("Socket");
	}

	// Make port number reuseable
	int opt = 1;
	result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*) & opt, sizeof(opt));
	if (result == -1) {
		error_die("setsockopt");
	}
	
	// Config server side address
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(*port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Bind socket
	bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));

	// Dynamic allocate port
	int nameLen = sizeof(server_addr);
	if (*port == 0) {
		if (getsockname(server_socket, (struct sockaddr*)&server_addr, &nameLen) < 0) {
			error_die("getsockname");
		}
		*port = server_addr.sin_port;
	}

	// Create listening queue
	if (listen(server_socket, 5) < 0){
		error_die("listen");
	}
	return server_socket;
}

// read a line and save them to buffer
int get_line(int sock, char* buff, int size) {
	char c = 0;
	int i = 0;
	while (i < size-1 && c != '\n') {
		int n = recv(sock, &c, 1, 0); //check if reading succeed
		if (n > 0) {
			if (c == '\r') {
				recv(sock, &c, 1, MSG_PEEK); //peek the incoming message if c = \r
				if (n > 0 && c == '\n') {
					recv(sock, &c, 1, 0);
				}
				else {
					break;
				}
			}
			buff[i++] = c;
		}
		else {
			break;
		}
	}
	buff[i] = 0;
	return i;
}


// Dealing with the thread that user created
DWORD WINAPI accept_request(LPVOID arg) {
	char buff[1024];

	int client = (SOCKET)arg;


	// Read a line of data 
	int numchars = get_line(client, buff, sizeof(buff));
	PRINTF(buff);

	char method[255];
	int i = 0;
	int j = 0;
	while (!isspace(buff[j]) && i < sizeof(method) - 1) {
		method[i++] = buff[j++];
	}
	method[i] = 0;
	PRINTF(method);
	
	//check if the method of the request is supported by this server

	// Resolve the path of resource file 
	// www.ekko.com/abc/index.html
	// GET /abc/index.html HTTP /1.1\n

	if (strcmp(method, "GET") && strcmp(method, "POST")) {

	}


	return 0;
}

int main() {
	unsigned short port = 8000;
	int server_sock = startUp(&port);
	cout << "httpd server started, listening to " << port << endl;

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);
	
	while (1) {
		int client_sock = accept(server_sock, (struct sockaddr*)& client_addr, &client_addr_len);
		if (client_sock == -1) {
			error_die("accept");
		}

		// Create a new thread
		// A process contains several threads
        
		DWORD threadId = 0;
		CreateThread(0, 0, accept_request, (void*)client_sock, 0, &threadId);


		

	}
	return 0;
}