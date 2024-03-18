#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include <iostream>
#include <tchar.h>
#include <WS2tcpip.h>
#include <regex>

#include "resource.h"

#pragma comment(lib, "WS2_32.lib")

const		char*	SERVER_ADDR = "127.0.0.1";
constexpr	short	SERVER_PORT = 4000;
const		int		BUFSIZE = 256;
std::string IP_addr;

HINSTANCE g_hInst;
LPCTSTR lpszClass = L"Test Client";
LPCTSTR lpszWindowName = L"Test Client";

HWND hWnd;

struct Object {
	int x, y;
	int wParam;
};

Object buf;
Object player[1];
int Num_Objects = 1;

void RenderBackground(PAINTSTRUCT ps, HDC hdc) {
	HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
	if (hBrush == NULL) {
		MessageBox(NULL, L"브러시를 생성할 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
		return;
	}

	RECT rect;
	for (int i = 0; i < 8; i += 2) {
		for (int j = 0; j < 8; j += 2) {
			SetRect(&rect, 80 * i, 80 * j, 80 * (i + 1), 80 * (j + 1));
			FillRect(hdc, &rect, hBrush);
		}
	}
	for (int i = 1; i < 8; i += 2) {
		for (int j = 1; j < 8; j += 2) {
			SetRect(&rect, 80 * i, 80 * j, 80 * (i + 1), 80 * (j + 1));
			FillRect(hdc, &rect, hBrush);
		}
	}

	DeleteObject(hBrush);
}

void RenderObject(PAINTSTRUCT ps, HDC hdc) {
	HBITMAP hBit;

	for (int i = 0; i < Num_Objects; ++i) {
		hBit = (HBITMAP)LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP1));
		if (hBit == NULL) {
			// 오류 처리
			MessageBox(NULL, L"이미지를 불러올 수 없습니다.", L"오류", MB_OK | MB_ICONERROR);
			return;
		}

		HDC memdc = CreateCompatibleDC(hdc);
		SelectObject(memdc, hBit);

		StretchBlt(hdc, player[0].x * 80, player[0].y * 80, 80, 80,
			memdc, 0, 0, 20, 20, SRCCOPY);
		DeleteDC(memdc);
	}
}

void Move(int* x, int* y, WPARAM wParam) {
	switch (wParam)
	{
	case 37:	//left
		if(player[0].x > 0) player[0].x -= 1;
		break;
	case 38:	//up
		if (player[0].y > 0) player[0].y -= 1;
		break;
	case 39:	//right
		if (player[0].x < 7) player[0].x += 1;
		break;
	case 40:	//down
		if (player[0].y < 7) player[0].y += 1;
		break;

	default:
		return;
	}

	player[0].wParam = reinterpret_cast<int>(&wParam);
}

void print_error(const char* msg, int err_no) {
	WCHAR* msg_buf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEPALI, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg_buf), 0, NULL);
	std::cout << msg;
	std::wcout << L" : ERROR : " << reinterpret_cast<WCHAR*>(&msg_buf);
	//while (true);
	LocalFree(msg_buf);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK Dialog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					LPSTR lpszCmdParam, int nCmdShow)
{
	MSG Message;
	WNDCLASSEX WndClass;

	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW, 0, 0, 655, 675, NULL,
		(HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	player[0].x = 0; player[0].y = 0; player[0].wParam = 0;
	buf = player[0];

	std::wcout.imbue(std::locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 0), &WSAData);

	SOCKET server_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	SOCKADDR_IN server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	if (IP_addr.empty()) { 
		inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr); 
	}
	else {
		//inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
		inet_pton(AF_INET, const_cast<char*>(IP_addr.c_str()), &server_addr.sin_addr);
	}

	if (connect(server_sock, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) != 0) {
		print_error("connect", WSAGetLastError());
	}

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
		
		WSABUF wsabuf[1];
		wsabuf[0].buf = reinterpret_cast<char*>(&buf);
		wsabuf[0].len = sizeof(Object) + 1;
		DWORD sent_size;

		WSASend(server_sock, wsabuf, 1, &sent_size, 0, NULL, NULL);

		wsabuf[0].len = BUFSIZE;
		DWORD recv_size; DWORD recv_flag = 0;
		int res = WSARecv(server_sock, wsabuf, 1, &recv_size, &recv_flag, NULL, NULL);
		if (res != 0) { print_error("WSARecv", WSAGetLastError()); }
		//else if (res == 0) { break; }

		player[0].x = buf.x;
		player[0].y = buf.y;
	}

	return Message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC = GetDC(hWnd);
	HBITMAP hBitmap;
	std::string child = "IP Input";
	//--- 메시지 처리하기

	switch (uMsg) {
	case WM_CREATE:
		DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Dialog_Proc);
		break;
	case WM_KEYDOWN:
		//Move(&player[0].x, &player[0].y, wParam);
		buf.wParam = wParam;
		InvalidateRect(hWnd, NULL, true);
		break;
	case WM_KEYUP:
		buf.wParam = 0;
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		RenderBackground(ps, hDC);
		RenderObject(ps, hDC);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
	//--- 위의 세 메시지 외의 나머지 메시지는 OS로
}

INT_PTR CALLBACK Dialog_Proc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR editText[20];
	TCHAR resultText[20] = L"127.0.0.1";
	std::wstring mem;

	switch (iMsg) {
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, resultText);		SetDlgItemText(hDlg, IDC_EDIT2, resultText);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK: //--- 버튼
			GetDlgItemText(hDlg, IDC_EDIT1, editText, 100);
			SetDlgItemText(hDlg, IDC_EDIT2, editText);			mem = (&editText[0]);			IP_addr.assign(mem.begin(), mem.end());
			break;
		case IDCANCEL: //--- 버튼			IP_addr.assign(mem.begin(), mem.end());
			EndDialog(hDlg, 0);
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return 0;
}