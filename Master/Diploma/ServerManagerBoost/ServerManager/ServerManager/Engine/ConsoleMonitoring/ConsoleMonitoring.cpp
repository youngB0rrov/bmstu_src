#include "ConsoleMonitoring.h"
#include <iostream>
#include <curses.h>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "../../Utils/CommandsHelper/CommandsHelper.h"

void ConsoleMonitoring::Run()
{
	Init();
	CleanupConsole();
}

void ConsoleMonitoring::Init()
{
	// Configuring ncurces
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	nodelay(stdscr, TRUE);
	keypad(stdscr, TRUE);

	start_color();
	init_pair(1, COLOR_BLACK, COLOR_GREEN);

	StartDrawLoop();
}

void ConsoleMonitoring::DrawTableHeader()
{
	int uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth;
	CalculateColumnsWidth(uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth);

	printw("Press \'q\' to quit monitoring tool");

	attron(COLOR_PAIR(1));

	mvprintw(1, 0, "%-*s %-*s %-*s %-*s %-*s",
		uuidColumnWidth, "UUID",
		uriColumnWidth, "Address",
		currentPlayersColumnWidth, "Current players",
		maxPlayersColumnWidth, "Max players",
		serverStateColumnWidth, "State"
	);

	attroff(COLOR_PAIR(1));
}

void ConsoleMonitoring::DrawServers()
{
	//ServerInfo serverInfo1;
	//serverInfo1.m_currentPlayers = 1;
	//serverInfo1.m_maxPlayers = 10;
	//serverInfo1.m_uuid = "asdfasdf";
	//serverInfo1.m_URI = "127.0.0.1:7777";
	//serverInfo1.m_serverState = ServerState::LOBBY;

	//std::vector<ServerInfo> testServers = {
	//	serverInfo1,
	//	serverInfo1,
	//	serverInfo1
	//};

	const std::vector<ServerInfo>& testServers = m_tcpServer->GetServerInstances();

	int row = 2;
	int uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth;
	CalculateColumnsWidth(uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth);

	for (auto& server : testServers)
	{
		mvprintw(row, 0, "%-*s %-*s %-*d %-*d %-*d",
			uuidColumnWidth, server.m_uuid.c_str(),
			uriColumnWidth, server.m_URI.c_str(),
			currentPlayersColumnWidth, server.m_currentPlayers,
			maxPlayersColumnWidth, server.m_maxPlayers,
			serverStateColumnWidth, server.m_serverState
		);

		row++;
	}
}

void ConsoleMonitoring::StartDrawLoop()
{

	while (true)
	{
		clear();
		DrawTableHeader();
		DrawServers();
		refresh();

		int ch = getch();
		if (ch == 'q')
		{
			break;
		}

		boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
}

void ConsoleMonitoring::CleanupConsole()
{
	endwin();
}

void ConsoleMonitoring::CalculateColumnsWidth(int& uuid, int& uri, int& currentPlayers, int& maxPlayers, int& serverState)
{
	int maxWindowWidth, maxWindowHeight;
	getmaxyx(stdscr, maxWindowHeight, maxWindowWidth);

	uuid = maxWindowWidth * 0.30;
	uri = maxWindowWidth * 0.20;
	currentPlayers = maxWindowWidth * 0.14;
	maxPlayers = maxWindowWidth * 0.14;
	serverState = maxWindowWidth * 0.20;
}
