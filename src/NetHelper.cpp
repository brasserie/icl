
#include "NetHelper.h"
#include "Protocol.h"
#include "JsonReader.h"
#include "Log.h"

namespace helper
{

/*****************************************************************************/
bool BasicClient::TestDiscard(const Deck &discard)
{
    return mDeck.TestDiscard(discard, mDog, mNbPlayers);
}
/*****************************************************************************/
Contract BasicClient::CalculateBid()
{
    int total = 0;
    Contract cont;

    UpdateStatistics();

    // Set points according to the card values
    if (mStats.bigTrump == true)
    {
        total += 9;
    }
    if (mStats.fool == true)
    {
        total += 7;
    }
    if (mStats.littleTrump == true)
    {
        if (mStats.trumps == 5)
        {
            total += 5;
        }
        else if (mStats.trumps == 6 || mStats.trumps == 7)
        {
            total += 7;
        }
        else if (mStats.trumps > 7)
        {
            total += 8;
        }
    }

    // Each trump is 1 point
    // Each major trump is 1 more point
    total += mStats.trumps * 2;
    total += mStats.majorTrumps * 2;
    total += mStats.kings * 6;
    total += mStats.queens * 3;
    total += mStats.knights * 2;
    total += mStats.jacks;
    total += mStats.weddings;
    total += mStats.longSuits * 5;
    total += mStats.cuts * 5;
    total += mStats.singletons * 3;
    total += mStats.sequences * 4;

    // We can decide the bid
    if (total <= 35)
    {
        cont = Contract::PASS;
    }
    else if (total >= 36  && total <= 50)
    {
        cont = Contract::TAKE;
    }
    else if (total >= 51  && total <= 65)
    {
        cont = Contract::GUARD;
    }
    else if (total >= 66  && total <= 75)
    {
        cont = Contract::GUARD_WITHOUT;
    }
    else
    {
        cont = Contract::GUARD_AGAINST;
    }
    return cont;
}
/*****************************************************************************/
void BasicClient::UpdateStatistics()
{
    mStats.Reset();
    mDeck.AnalyzeTrumps(mStats);
    mDeck.AnalyzeSuits(mStats);
}
/*****************************************************************************/
Card BasicClient::Play()
{
    Card c;

    for (Deck::ConstIterator it = mDeck.Begin(); it != mDeck.End(); ++it)
    {
        c = (*it);
        if (IsValid(c) == true)
        {
            break;
        }
    }
    return c;
}
/*****************************************************************************/
bool BasicClient::IsValid(const Card &c)
{
    return mDeck.CanPlayCard(c, mCurrentTrick);
}
/*****************************************************************************/
Deck BasicClient::AutoDiscard()
{
    Deck discard = mDeck.AutoDiscard(mDog, mNbPlayers);
    mDeck.RemoveDuplicates(discard);
    return discard;
}
/*****************************************************************************/
void BasicClient::BuildBid(Contract c, bool slam, std::vector<helper::Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "ReplyBid");
    obj.AddValue("contract", c.ToString());
    obj.AddValue("slam", slam);

    out.push_back(helper::Reply(mTableId, obj));
}
/*****************************************************************************/
void BasicClient::SendHandle(const Deck &handle, std::vector<helper::Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "Handle");
    obj.AddValue("handle", handle.ToString());

    out.push_back(helper::Reply(mTableId, obj));
}
/*****************************************************************************/
void BasicClient::SendDiscard(const Deck &discard, std::vector<helper::Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "Discard");
    obj.AddValue("discard", discard.ToString());

    out.push_back(helper::Reply(mTableId, obj));
}
/*****************************************************************************/
void BasicClient::SendCard(Card c, std::vector<Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "Card");
    obj.AddValue("card", c.ToString());

    out.push_back(helper::Reply(mTableId, obj));
}
/*****************************************************************************/
void BasicClient::BuildChangeNickname(std::vector<helper::Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "ReqChangeNickname");
    obj.AddValue("nickname", mNickName);

    out.push_back(helper::Reply(Protocol::LOBBY_UID, obj));
}
/*****************************************************************************/
void BasicClient::JoinTable(uint32_t tableId, std::vector<Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "RequestJoinTable");
    obj.AddValue("table_id", tableId);

    out.push_back(helper::Reply(Protocol::LOBBY_UID, obj));
}
/*****************************************************************************/
void BasicClient::Sync(const std::string &step, std::vector<Reply> &out)
{
    JsonObject obj;

    obj.AddValue("cmd", "Ack");
    obj.AddValue("step", step);

    out.push_back(helper::Reply(mTableId, obj));
}
/*****************************************************************************/
BasicClient::Event BasicClient::Decode(uint32_t src_uuid, uint32_t dest_uuid, const std::string &arg, IContext &ctx, std::vector<helper::Reply> &out)
{
    BasicClient::Event event = BAD_EVENT;
    JsonReader reader;
    JsonValue json;

    if (!reader.ParseString(json, arg))
    {
        TLogNetwork("Not a JSON data");
        return JSON_ERROR;
    }

    std::string cmd = json.FindValue("cmd").GetString();

    if (cmd == "RequestLogin")
    {
        ctx.Initialize();

        mUuid = static_cast<std::uint32_t>(json.FindValue("uuid").GetInteger());

        JsonObject obj;

        obj.AddValue("cmd", "ReplyLogin");
        obj.AddValue("nickname", mNickName);

        out.push_back(helper::Reply(Protocol::LOBBY_UID, obj));

        event = REQ_LOGIN;
    }
    else if (cmd == "AccessGranted")
    {
        JsonArray tables = json.FindValue("tables").GetArray();

        for (std::uint32_t i = 0U; i < tables.Size(); i++)
        {
           JsonObject table = tables.GetEntry(i).GetObject();
           std::uint32_t uuid = static_cast<std::uint32_t>(table.GetValue("uuid").GetInteger());
           std::string name = table.GetValue("name").GetString();
           ctx.AddTable(name, uuid);
        }

        event = ACCESS_GRANTED;
    }
    else if (cmd == "ChatMessage")
    {
        IContext::Message msg;
        msg.src = static_cast<std::uint32_t>(json.FindValue("source").GetInteger());
        msg.dst = static_cast<std::uint32_t>(json.FindValue("target").GetInteger());
        msg.msg = json.FindValue("message").GetString();

        event = MESSAGE;
    }
    else if (cmd == "PlayerList")
    {
        JsonArray players = json.FindValue("players").GetArray();

        for (std::uint32_t i = 0U; i < players.Size(); i++)
        {
            IContext::Member member;
            JsonObject player = players.GetEntry(i).GetObject();

            std::uint32_t uuid = static_cast<std::uint32_t>(player.GetValue("uuid").GetInteger());
            member.table = static_cast<std::uint32_t>(player.GetValue("table").GetInteger());
            member.nickname = player.GetValue("nickname").GetString();
            std::string event = player.GetValue("event").GetString();
            ctx.UpdateMember(uuid, member, event);
        }

        event = PLAYER_LIST;
    }
    else if (cmd == "ReplyJoinTable")
    {
        mPlace = Place(json.FindValue("place").GetString());
        mTableId = static_cast<std::uint32_t>(json.FindValue("table_id").GetInteger());
        mNbPlayers = static_cast<std::uint32_t>(json.FindValue("size").GetInteger());

        Sync("JoinTable", out);
    }
    else if (cmd == "RequestQuitTable")
    {
        event = QUIT_TABLE;
    }
    else if (cmd == "NewGame")
    {
        mGame.Set(json.FindValue("mode").GetString());
        Sync("Ready", out);

        event = NEW_GAME;
    }
    else if (cmd == "NewDeal")
    {
        mDeck.SetCards(json.FindValue("cards").GetString());
        Sync("NewDeal", out);

        event = NEW_DEAL;
    }
    else if (cmd == "RequestBid")
    {
        mBid.taker = Place(json.FindValue("place").GetString());
        mBid.contract = Contract(json.FindValue("contract").GetString());

        event = REQ_BID;
    }
    else if (cmd == "ShowBid")
    {
        mBid.taker = Place(json.FindValue("place").GetString());
        mBid.contract = Contract(json.FindValue("contract").GetString());
        mBid.slam = json.FindValue("slam").GetBool();

        Sync(cmd, out);

        event = SHOW_BID;
    }
    else if (cmd == "AllPassed")
    {
        Sync("AllPassed", out);

        event = ALL_PASSED;
    }
    else if (cmd == "ShowDog")
    {
        mDog.SetCards(json.FindValue("dog").GetString());
        Sync(cmd, out);

        event = SHOW_DOG;
    }
    else if (cmd == "BuildDiscard")
    {
        event = BUILD_DISCARD;
    }
    else if (cmd == "StartDeal")
    {
        mBid.taker = Place(json.FindValue("taker").GetString());
        mBid.contract = Contract(json.FindValue("contract").GetString());
        mBid.slam = json.FindValue("slam").GetBool();

        UpdateStatistics();
        mCurrentTrick.Clear();

        Sync(cmd, out);

        event = START_DEAL;
    }
    else if (cmd == "AskForHandle")
    {
        event = ASK_FOR_HANDLE;
    }
    else if (cmd == "ShowHandle")
    {
        Place place = Place(json.FindValue("place").GetString());
        mHandle.SetCards(json.FindValue("handle").GetString());

        Team team(Team::DEFENSE);
        if (place == mBid.taker)
        {
            team = Team::ATTACK;
        }
        mHandle.SetOwner(team);

        Sync("Handle", out);

        event = SHOW_HANDLE;
    }
    else if (cmd == "ShowCard")
    {
        mCurrentPlayer = Place(json.FindValue("place").GetString());
        Card card = Card(json.FindValue("card").GetString());

        mCurrentTrick.Append(card);

        // We have seen the card, let's inform the server about that
        Sync("Card", out);

        event = SHOW_CARD;
    }
    else if (cmd == "PlayCard")
    {
        mCurrentPlayer = Place(json.FindValue("place").GetString());

        event = PLAY_CARD;
    }
    else if (cmd == "EndOfTrick")
    {
        mCurrentPlayer = Place(json.FindValue("place").GetString());
        mCurrentTrick.Clear();

        event = END_OF_TRICK;
    }
    else if (cmd == "EndOfDeal")
    {
        mPoints.pointsAttack = json.FindValue("points").GetInteger();
        mPoints.oudlers = json.FindValue("oudlers").GetInteger();
        mPoints.littleEndianOwner = static_cast<std::uint8_t>(json.FindValue("little_bonus").GetInteger());
        mPoints.handlePoints = json.FindValue("handle_bonus").GetInteger();
        mPoints.slamDone = json.FindValue("slam_bonus").GetBool();

        JsonObject deal = json.FindValue("deal").GetObject();

        ctx.SetResult(deal);

        Sync(cmd, out);
        event = END_OF_DEAL;
    }
    else if (cmd == "EndOfGame")
    {
        mCurrentPlayer = Place(json.FindValue("winner").GetString());
        event = END_OF_GAME;
    }
    else
    {
        TLogError("Command not supported: " + cmd);
    }

    return event;
}


} // End of namespace helper
