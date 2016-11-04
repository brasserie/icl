
#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include <cstdint>
#include <iostream>
#include <Bot.h>

#include "tst_tarot_protocol.h"
#include "Lobby.h"

TarotProtocol::TarotProtocol()
{
}


bool Dump(const std::vector<helper::Reply> &reply)
{
    bool endOfGame = false;
    for (std::uint32_t i = 0U; i < reply.size(); i++)
    {
        std::cout << "------------------------------------" << std::endl;
        std::cout << "Destination: " << (int)reply[i].dest  << std::endl;
        std::string data = reply[i].data.ToString(0U);
        std::cout << data << std::endl;

        if (data.find("EndOfGame") != std::string::npos)
        {
            endOfGame = true;
        }
    }
    return endOfGame;
}


void TarotProtocol::TestPlayerJoinAndQuit()
{
    std::vector<std::string> tables;
    tables.push_back("test");
    Lobby lobby;

    lobby.Initialize("test", tables);

    std::vector<helper::Reply> reply;

    std::uint32_t src = Protocol::INVALID_UID;
    std::uint32_t dest = Protocol::LOBBY_UID;

    JsonObject json;

    src = lobby.AddUser(reply);
    Dump(reply);
    reply.clear();

    json.AddValue("cmd", "ReplyLogin");
    json.AddValue("nickname", "Casimir");

    QCOMPARE(lobby.Decode(src, dest, json.ToString(0U), reply), true);
    Dump(reply);
    reply.clear();
}

void TarotProtocol::TestBotsFullGame()
{
    std::vector<std::string> tables;
    tables.push_back("test");
    Lobby lobby;

    lobby.Initialize("test", tables);

    std::vector<helper::Reply> lobby_data;

    struct BotManager
    {
        std::vector<helper::Reply> reply;
        Bot bot;
        std::uint32_t uuid;
    };

    BotManager bots[4];
    std::string names[4];
    names[0] = "riri";
    names[1] = "fifi";
    names[2] = "loulou";
    names[3] = "tata";

    std::uint32_t tableId = Protocol::TABLES_UID;

    // Initialize & Register the 4 bots on the lobby
    for (int i = 0; i < 4; i++)
    {
        bots[i].uuid = lobby.AddUser(lobby_data);
        bots[i].bot.SetUser(names[i], "");
        bots[i].bot.SetAiScript(Util::ExecutablePath() + "/../../bin/aicontest/ai.zip");
        bots[i].bot.SetTableToJoin(tableId);
    }

    Dump(lobby_data);

    // We run N games in a loop
    int gameCounter = 0;
    while(gameCounter < 10)
    {
        // For all bots
        for (int i = 0; i < 4; i++)
        {
            bots[i].reply.clear();

            std::cout << "------------------------------------" << std::endl;
            std::cout << "Bot " << bots[i].bot.GetPlace().ToString() << " cards: " << bots[i].bot.GetDeck() << std::endl;

            // Send data to that bot if any
            for (std::uint32_t j = 0U; j < lobby_data.size(); j++)
            {
                if ((lobby_data[j].dest == bots[i].uuid) ||
                    (lobby_data[j].dest == Protocol::LOBBY_UID) ||
                    (lobby_data[j].dest == bots[i].bot.GetCurrentTable()))
                {
                    bots[i].bot.Decode(Protocol::LOBBY_UID, lobby_data[j].dest, lobby_data[j].data.ToString(0U), bots[i].reply);
                }
            }

            Dump(bots[i].reply);
        }

        lobby_data.clear();
        // For all bots
        for (int i = 0; i < 4; i++)
        {
            // Send bot replies to the lobby (destination should be always the lobby or a table)
            for (std::uint32_t j = 0U; j < bots[i].reply.size(); j++)
            {
                QCOMPARE(lobby.Decode(bots[i].uuid, bots[i].reply[j].dest, bots[i].reply[j].data.ToString(0U), lobby_data), true);
            }
        }
        if (Dump(lobby_data))
        {
            gameCounter++;
        }
    }
}
