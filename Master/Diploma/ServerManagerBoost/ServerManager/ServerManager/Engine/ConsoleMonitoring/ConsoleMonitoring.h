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

enum class SortColumn
{
	UUID,
	URI,
	CURRENT_PLAYERS,
	MAX_PLAYERS,
	STATE
};

class ConsoleMonitoring
{
public:
	ConsoleMonitoring(TcpServer* tcpServer) : m_tcpServer(tcpServer) 
	{
		m_sortColumn = SortColumn::CURRENT_PLAYERS;
		m_bDesendingSort = true;
		m_bIsSerching = false;
		m_searchQueryString = "";
	};


	void Run();

private:
	void Init();
	void DrawTableHeader();
	void DrawServers();
	void StartDrawLoop();
	void DrawFooter();
	void CleanupConsole();
	void UpdateScreen();
	void ToggleSortColumn();
	void ToggleSortDirection();
	void ToggleSearchMode();
	void FilterServers(std::vector<ServerInfo>& servers);

	static void CalculateColumnsWidth(int& uuid, int& uri, int& currentPlayers, int& maxPlayers, int& serverState);

	TcpServer* m_tcpServer;
	SortColumn m_sortColumn;
	bool m_bDesendingSort;
	bool m_bIsSerching;
	std::string m_searchQueryString;
};