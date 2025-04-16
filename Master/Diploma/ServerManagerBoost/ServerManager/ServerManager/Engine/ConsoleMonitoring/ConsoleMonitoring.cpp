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
	//ServerInfo serverInfo1, serverInfo2, serverInfo3, serverInfo4, serverInfo5, serverInfo6, serverInfo7;
	//serverInfo1.m_currentPlayers = 1;
	//serverInfo1.m_maxPlayers = 10;
	//serverInfo1.m_uuid = "886a0c1f-5095-4724-b7a6-c8067456b475";
	//serverInfo1.m_URI = "192.168.1.12:7777";
	//serverInfo1.m_serverState = ServerState::LOBBY;

	//serverInfo2.m_currentPlayers = 5;
	//serverInfo2.m_maxPlayers = 10;
	//serverInfo2.m_uuid = "75295fae-715f-40e7-aaff-f69ab9ebe4d9";
	//serverInfo2.m_URI = "192.168.1.12:7778";
	//serverInfo2.m_serverState = ServerState::MATCH_STARTING;

	//serverInfo3.m_currentPlayers = 3;
	//serverInfo3.m_maxPlayers = 25;
	//serverInfo3.m_uuid = "e9f53c2b-97fd-428c-9430-0bb3cf497805";
	//serverInfo3.m_URI = "192.168.1.12:7779";
	//serverInfo3.m_serverState = ServerState::MATCH_IN_PROGRESS;

	//serverInfo4.m_currentPlayers = 7;
	//serverInfo4.m_maxPlayers = 12;
	//serverInfo4.m_uuid = "53b64d04-31f6-4251-8c61-0c6a683fce2e";
	//serverInfo4.m_URI = "192.168.1.12:7780";
	//serverInfo4.m_serverState = ServerState::MATCH_IN_PROGRESS;

	//serverInfo5.m_currentPlayers = 2;
	//serverInfo5.m_maxPlayers = 30;
	//serverInfo5.m_uuid = "5819a66a-f4e8-4f68-8a83-a2f113a1a70b";
	//serverInfo5.m_URI = "192.168.1.12:7781";
	//serverInfo5.m_serverState = ServerState::SHTUTTING_DOWN;

	//serverInfo6.m_currentPlayers = 1;
	//serverInfo6.m_maxPlayers = 10;
	//serverInfo6.m_uuid = "90660580-2c9e-4ad9-bb1e-1c942b45e5bb";
	//serverInfo6.m_URI = "192.168.1.12:7782";
	//serverInfo6.m_serverState = ServerState::LOBBY;

	//serverInfo7.m_currentPlayers = 4;
	//serverInfo7.m_maxPlayers = 20;
	//serverInfo7.m_uuid = "cf778f23-1bc5-48c2-97f5-ac628302ccd3";
	//serverInfo7.m_URI = "192.168.1.12:7783";
	//serverInfo7.m_serverState = ServerState::MATCH_IN_PROGRESS;

	//std::vector<ServerInfo> runningServers = {
	//	serverInfo1,
	//	serverInfo2,
	//	serverInfo3,
	//	serverInfo4,
	//	serverInfo5,
	//	serverInfo6,
	//	serverInfo7
	//};

	std::vector<ServerInfo> runningServers = m_tcpServer->GetServerInstances();

	std::sort(runningServers.begin(), runningServers.end(), [this](const ServerInfo& A, const ServerInfo& B)
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

	if (m_bIsSerching)
	{
		FilterServers(runningServers);
	}

	int row = 1;
	int uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth;
	CalculateColumnsWidth(uuidColumnWidth, uriColumnWidth, currentPlayersColumnWidth, maxPlayersColumnWidth, serverStateColumnWidth);

	for (auto& server : runningServers)
	{
		mvprintw(row, 0, "%-*s %-*s %-*d %-*d %-*s",
			uuidColumnWidth, server.m_uuid.c_str(),
			uriColumnWidth, server.m_URI.c_str(),
			currentPlayersColumnWidth, server.m_currentPlayers,
			maxPlayersColumnWidth, server.m_maxPlayers,
			serverStateColumnWidth, ToString(server.m_serverState).c_str()
		);

		row++;
	}
}

void ConsoleMonitoring::StartDrawLoop()
{
	wtimeout(stdscr, 1000); // ќжидание ввода до 1 секунды

	while (true)
	{
		UpdateScreen();

		int ch = getch();
		if (ch == 265) // F1
		{
			ToggleSearchMode();
		}
		else if (ch == 266) // F2
		{
			ToggleSortColumn();
		}
		else if (ch == 267)
		{
			ToggleSortDirection(); // F3
		}
		else if (ch == 274) // F10
		{
			break;
		}
		else if (ch == 27) // Esc
		{
			ToggleSearchMode();
		}
		else if (ch == 330 || ch == 8) // Backspace или Delete дл€ удалени€ символа
		{
			if (!m_searchQueryString.empty())
			{
				m_searchQueryString.pop_back();
			}
		}
		else if (ch >= 32 && ch <= 126) // ¬вод любого символа с клавиатуры
		{
			if (m_bIsSerching)
			{
				m_searchQueryString += static_cast<char>(ch);
			}
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

		mvprintw(maxWindowHeight, (int)offset, keyPair.first.c_str());
		offset += keyLength;
		attron(COLOR_PAIR(2));

		mvprintw(maxWindowHeight, (int)offset, keyPair.second.c_str());
		attroff(COLOR_PAIR(2));
		offset += valueLength;
	}

	if (m_bIsSerching)
	{
		mvprintw(maxWindowHeight, (int)offset + 1, "Searching (ESC/F1 to cancel): %s ", m_searchQueryString.c_str());
	}
	else
	{
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

		mvprintw(maxWindowHeight, (int)offset + 1, "Sorting: %s (%s)", sortColumntText.c_str(), sortDirectionText.c_str());
	}
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

void ConsoleMonitoring::ToggleSearchMode()
{
	m_bIsSerching = !m_bIsSerching;
	m_searchQueryString = "";
}

void ConsoleMonitoring::FilterServers(std::vector<ServerInfo>& servers)
{
	if (m_searchQueryString.empty()) return;

	servers.erase(std::remove_if(servers.begin(), servers.end(), [this](const ServerInfo& info)
	{
		return info.m_uuid.find(m_searchQueryString) == std::string::npos &&
			info.m_URI.find(m_searchQueryString) == std::string::npos &&
			std::to_string(info.m_currentPlayers).find(m_searchQueryString) == std::string::npos &&
			std::to_string(info.m_maxPlayers).find(m_searchQueryString) == std::string::npos &&
			ToString(info.m_serverState).find(m_searchQueryString) == std::string::npos;
	}), servers.end());
}

void ConsoleMonitoring::CalculateColumnsWidth(int& uuid, int& uri, int& currentPlayers, int& maxPlayers, int& serverState)
{
	int maxWindowWidth, maxWindowHeight;
	getmaxyx(stdscr, maxWindowHeight, maxWindowWidth);

	uuid = (int)(maxWindowWidth * 0.33f);
	uri = (int)maxWindowWidth * 0.15f;
	currentPlayers = (int)maxWindowWidth * 0.15f;
	maxPlayers = (int)maxWindowWidth * 0.12f;
	serverState = (int)maxWindowWidth * 0.20f;
}
