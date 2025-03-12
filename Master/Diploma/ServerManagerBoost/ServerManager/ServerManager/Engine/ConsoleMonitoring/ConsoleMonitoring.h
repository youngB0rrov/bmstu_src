#pragma once

#ifdef _WIN32
	#if __has_include(<curses.h>)
		#include <curses.h>
	#else
		#pragma message("PDCurses not found, skipping include")
	#endif
#else
	#if __has_include(<ncurses.h>)
		#include <ncurses.h>
	#else
		#pragma message("ncurses not found, skipping include")
	#endif
#endif

#include "../TcpServer/TcpServer.h"
#include "../../Data/Enums/ServerState.h"

class ConsoleMonitoring
{
public:
	ConsoleMonitoring(TcpServer* tcpServer) : m_tcpServer(tcpServer) {};
	void Run();

private:
	void Init();
	void DrawTableHeader();
	void DrawServers();
	void StartDrawLoop();
	void CleanupConsole();

	static void CalculateColumnsWidth(int& uuid, int& uri, int& currentPlayers, int& maxPlayers, int& serverState);

	TcpServer* m_tcpServer;
};