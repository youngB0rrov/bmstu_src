#include "ConsoleMonitoring.h"
#include <iostream>
#include <curses.h>
#include <boost/thread.hpp>
#include <algorithm>
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
	init_pair(2, COLOR_BLACK, COLOR_BLUE);

	StartDrawLoop();
}

void ConsoleMonitoring::DrawTableHeader()
{
	int uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth;
	CalculateColumnsWidth(uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth);

	attron(COLOR_PAIR(1));

	mvprintw(0, 0, "%-*s %-*s %-*s %-*s %-*s",
		uuidColumnWidth, m_sortColumn == SortColumn::UUID ? "[UUID]" : "UUID",
		uriColumnWidth, m_sortColumn == SortColumn::URI ? "[Address]" : "Address",
		currentPlayersColumnWidth, m_sortColumn == SortColumn::CURRENT_PLAYERS ? "[Current players]" : "Current players",
		maxPlayersColumnWidth, m_sortColumn == SortColumn::MAX_PLAYERS ? "[Max players]" : "Max players",
		serverStateColumnWidth, m_sortColumn == SortColumn::STATE ? "[State]" : "State"
	);

	attroff(COLOR_PAIR(1));
}

void ConsoleMonitoring::DrawServers()
{
	ServerInfo serverInfo1, serverInfo2, serverInfo3;
	serverInfo1.m_currentPlayers = 1;
	serverInfo1.m_maxPlayers = 10;
	serverInfo1.m_uuid = "test123";
	serverInfo1.m_URI = "127.0.0.1:7777";
	serverInfo1.m_serverState = ServerState::LOBBY;

	serverInfo2.m_currentPlayers = 5;
	serverInfo2.m_maxPlayers = 10;
	serverInfo2.m_uuid = "test4321";
	serverInfo2.m_URI = "127.0.0.1:7778";
	serverInfo2.m_serverState = ServerState::MATCH_STARTING;

	serverInfo3.m_currentPlayers = 3;
	serverInfo3.m_maxPlayers = 20;
	serverInfo3.m_uuid = "qwerty321123";
	serverInfo3.m_URI = "127.0.0.1:7779";
	serverInfo3.m_serverState = ServerState::MATCH_IN_PROGRESS;

	std::vector<ServerInfo> testServers = {
		serverInfo1,
		serverInfo2,
		serverInfo3
	};

	//const std::vector<ServerInfo>& testServers = m_tcpServer->GetServerInstances();

	std::sort(testServers.begin(), testServers.end(), [this](const ServerInfo& A, const ServerInfo& B)
	{
		switch (m_sortColumn)
		{
			case SortColumn::UUID:
				return m_bDesendingSort ? A.m_uuid > B.m_uuid : A.m_uuid < B.m_uuid;
			case SortColumn::URI:
				return m_bDesendingSort ? A.m_URI> B.m_URI : A.m_URI < B.m_URI;
			case SortColumn::CURRENT_PLAYERS:
				return m_bDesendingSort ? A.m_currentPlayers > B.m_currentPlayers : A.m_currentPlayers < B.m_currentPlayers;
			case SortColumn::MAX_PLAYERS:
				return m_bDesendingSort ? A.m_maxPlayers > B.m_maxPlayers : A.m_maxPlayers < B.m_maxPlayers;
			case SortColumn::STATE:
				return m_bDesendingSort ? A.m_serverState > B.m_serverState : A.m_serverState < B.m_serverState;
			default:
				return true;
		}
	});

	int row = 1;
	int uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth;
	CalculateColumnsWidth(uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth);

	for (auto& server : testServers)
	{
		mvprintw(row, 0, "%-*s %-*s %-*d %-*d %-*s",
			uuidColumnWidth, server.m_uuid.c_str(),
			uriColumnWidth, server.m_URI.c_str(),
			currentPlayersColumnWidth, server.m_currentPlayers,
			maxPlayersColumnWidth, server.m_maxPlayers,
			serverStateColumnWidth, ToString(server.m_serverState)
		);

		row++;
	}
}

void ConsoleMonitoring::StartDrawLoop()
{
	timeout(1000); // ќжидание ввода до 1 секунды

	while (true)
	{
		UpdateScreen();

		int ch = getch();
		if (ch == 266) // F2
		{
			ToggleSortColumn();
		}
		if (ch == 267)
		{
			ToggleSortDirection(); // F3
		}
		if (ch == 274) // F10
		{
			break;
		}

		//boost::this_thread::sleep(boost::posix_time::seconds(1));
	}
}

void ConsoleMonitoring::DrawFooter()
{
	int maxWindowWidth, maxWindowHeight;
	getmaxyx(stdscr, maxWindowHeight, maxWindowWidth);
	maxWindowHeight -= 1;
	size_t offset = 0;

	static const std::vector<std::pair<std::string, std::string>> keyMap =
	{
		{ "F1", "Search" },
		{ "F2", "Change sort column" },
		{ "F3", "Change sort direction" },
		{ "F10", "Quit" }
	};

	for (auto& keyPair : keyMap)
	{
		size_t keyLength = keyPair.first.length();
		size_t valueLength = keyPair.second.length();

		mvprintw(maxWindowHeight, offset, keyPair.first.c_str());
		offset += keyLength;
		attron(COLOR_PAIR(2));

		mvprintw(maxWindowHeight, offset, keyPair.second.c_str());
		attroff(COLOR_PAIR(2));
		offset += valueLength;
	}

	std::string sortColumntText;
	std::string sortDirectionText;

	switch (m_sortColumn)
	{
		case SortColumn::UUID:
			sortColumntText = "UUID";
			break;
		case SortColumn::URI:
			sortColumntText = "URI";
			break;
		case SortColumn::CURRENT_PLAYERS:
			sortColumntText = "Current players";
			break;
		case SortColumn::MAX_PLAYERS:
			sortColumntText = "Max players";
			break;
		case SortColumn::STATE:
			sortColumntText = "Server state";
			break;
		default:
			sortColumntText = "UNDEFINED";
	}

	sortDirectionText = m_bDesendingSort ? "Descending" : "Ascending";

	mvprintw(maxWindowHeight, offset + 1, "Sorting: %s (%s)", sortColumntText.c_str(), sortDirectionText.c_str());
}

void ConsoleMonitoring::CleanupConsole()
{
	endwin();
}

void ConsoleMonitoring::UpdateScreen()
{
	clear();
	DrawTableHeader();
	DrawServers();
	DrawFooter();
	refresh();
}

void ConsoleMonitoring::ToggleSortColumn()
{
	int column = static_cast<int>(m_sortColumn);
	column = (column + 1) % 5;
	m_sortColumn = static_cast<SortColumn>(column);

	UpdateScreen();
}

void ConsoleMonitoring::ToggleSortDirection()
{
	m_bDesendingSort = !m_bDesendingSort;

	UpdateScreen();
}

void ConsoleMonitoring::CalculateColumnsWidth(int& uuid, int& uri, int& currentPlayers, int& maxPlayers, int& serverState)
{
	int maxWindowWidth, maxWindowHeight;
	getmaxyx(stdscr, maxWindowHeight, maxWindowWidth);

	uuid = maxWindowWidth * 0.29;
	uri = maxWindowWidth * 0.20;
	currentPlayers = maxWindowWidth * 0.14;
	maxPlayers = maxWindowWidth * 0.14;
	serverState = maxWindowWidth * 0.20;
}
