#include <iostream>
#include <WS2tcpip.h>

#pragma comment(lib, "WS2_32.lib")

constexpr	short	SERVER_PORT = 4000;
const		int		BUFSIZE		= 256;

struct Data {
	int x, y;
	int wParam;
};

Data buf;

void print_error(const char* msg, int err_no) {
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no, 
		MAKELANGID(LANG_NEPALI, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : ERROR : " << reinterpret_cast<WCHAR*>(&msg_buf);
	//while (true);
	LocalFree(msg_buf);
}

void Move(int* x, int* y, WPARAM wParam) {
	switch (wParam)
	{
	case 37:	//left
		if (buf.x > 0) buf.x -= 1;
		break;
	case 38:	//up
		if (buf.y > 0) buf.y -= 1;
		break;
	case 39:	//right
		if (buf.x < 7) buf.x += 1;
		break;
	case 40:	//down
		if (buf.y < 7) buf.y += 1;
		break;

	default:
		return;
	}

	buf.wParam = reinterpret_cast<int>(&wParam);
}

int main()
{
	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	SOCKADDR_IN server_addr;
	server_addr.sin_family	= AF_INET;
	server_addr.sin_port	= htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server_sock, SOMAXCONN);
	int addr_size = sizeof(server_addr);
	SOCKET client_sock = WSAAccept(server_sock, reinterpret_cast<sockaddr*>(&server_addr),
		&addr_size, NULL, 0);

	while (true) {
		WSABUF wsabuf[1];
		wsabuf[0].buf = reinterpret_cast<char*>(&buf);
		wsabuf[0].len = BUFSIZE;
		DWORD recv_size; DWORD recv_flag = 0;

		int res = WSARecv(client_sock, wsabuf, 1, &recv_size, &recv_flag, NULL, NULL);
		//std::cout << "A: " << buf.x << ", " << buf.y << ", " << buf.wParam << std::endl;
		if (res != 0) { print_error("WSARecv", WSAGetLastError()); }
		//else if (res == 0) { break; }

		Move(&buf.x, &buf.y, buf.wParam);

		DWORD sent_size;
		wsabuf[0].len = sizeof(Data) + 1;
		WSASend(client_sock, wsabuf, 1, &sent_size, 0, NULL, NULL);
		//std::cout << "B: " << buf.x << ", " << buf.y << ", " << buf.wParam << std::endl;
	}

	closesocket(server_sock);
	WSACleanup();
}