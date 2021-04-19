#include "online_service.h"

using namespace std;

OnlineService::OnlineService(){};

bool OnlineService::parseJson(string json_str, Json::Value *json_value)
{
    json_reader_.parse(json_str, *json_value);
}

bool OnlineService::printCurrentStat()
{
    cout << "[DEBUG] User:" << endl;
    for (const auto &user : user_map_)
    {
        cout << "   " << user.first << "," << user.second.id << "," << user.second.pin << endl;
    }

    cout << "[DEBUG] Room:" << endl;
    for (const auto &room : room_map_)
    {
        cout << "   " << room.first << "," << room.second.id << "," << room.second.owner << ","
             << room.second.is_locked << endl;
        for (const auto &mem : room.second.members)
        {
            cout << "        " << mem << endl;
        }
    }
}

bool OnlineService::isUserVaild(string user_id, string pin)
{
    cout << "[DEBUG] user :" << user_id << "," << pin << endl;
    return user_map_.count(user_id) != 0 && user_map_[user_id].pin == pin;
}

bool OnlineService::sendFailResponse(string user_id, string text)
{
    Json::Value respond;
    respond["ctl"] = static_cast<int>(ControlType::STATUS);
    respond["status"] = FAIL;
    respond["hint"] = text;
    return c2s_adapter_map_[user_id]->sendTextMessage(respond.toStyledString());
}

bool OnlineService::sendFailResponse(int fd, string text)
{
    Json::Value respond;
    respond["ctl"] = static_cast<int>(ControlType::STATUS);
    respond["status"] = FAIL;
    respond["hint"] = text;
    main_adapter_.setSocket(fd);
    return main_adapter_.sendTextMessage(respond.toStyledString());
}

bool OnlineService::sendSuccessResponse(string user_id, const Json::Value &extra_data)
{
    Json::Value respond;
    respond["ctl"] = static_cast<int>(ControlType::STATUS);
    respond["status"] = SUCCESS;
    respond["data"] = extra_data;
    return c2s_adapter_map_[user_id]->sendTextMessage(respond.toStyledString());
}

bool OnlineService::sendSuccessResponse(string user_id)
{
    Json::Value respond;
    respond["ctl"] = static_cast<int>(ControlType::STATUS);
    respond["status"] = SUCCESS;
    return c2s_adapter_map_[user_id]->sendTextMessage(respond.toStyledString());
}

bool OnlineService::process(string json_request, int fd)
{
    printCurrentStat();
    Json::Value json_value;
    parseJson(json_request, &json_value);
    int ctl_type = json_value["ctl"].asInt();
    switch (static_cast<ControlType>(ctl_type))
    {
    case ControlType::LOGIN:
    {
        string user_id = json_value["user_id"].asString();
        string pin = json_value["pin"].asString();
        cout << "USER:" << user_id << " try to login." << endl;

        if (user_map_.count(user_id) != 0 &&
            user_map_[user_id].pin != pin)
        {
            return sendFailResponse(fd, "user exist with different pin");
        }
        // success
        UserInfo user = {user_id, pin};
        user_map_[user_id] = user;
        c2s_adapter_map_[user_id].reset(new MessageAdapter(fd));

        // get user status
        Json::Value user_status;
        for (const auto &room_pair : room_map_)
        {
            if (room_pair.second.members.count(user_id) > 0)
            {
                user_status["room_id"] = room_pair.first;
                break;
            }
        }
        return sendSuccessResponse(user_id, user_status);
        break;
    }
    case ControlType::ENTER:
    {
        string user_id = json_value["user_id"].asString();
        string pin = json_value["pin"].asString();
        string room_id = json_value["room_id"].asString();

        cout << "USER:" << user_id << " try to enter ROOM:" << room_id << "." << endl;

        if (!isUserVaild(user_id, pin))
        {
            return sendFailResponse(user_id, "user info not found");
        }
        if (room_map_.count(room_id) != 0 &&
            room_map_[room_id].is_locked == true &&
            room_map_[room_id].owner != user_id)
        {
            return sendFailResponse(user_id, "room locked");
        }

        // success
        if (room_map_.count(room_id) == 0)
        {
            RoomInfo room = {room_id, user_id, false, {}};
            room_map_[room_id] = room;
        }
        room_map_[room_id].members.insert(user_id);

        Json::Value room_map;
        for (const auto &room_pair : room_map_)
        {
            Json::Value room_info;
            room_info["id"] = room_pair.second.id;
            room_info["owner"] = room_pair.second.owner;
            room_info["is_locked"] = room_pair.second.is_locked;
            room_map[room_pair.first] = room_info;
        }
        return sendSuccessResponse(user_id, room_map);
        break;
    }
    case ControlType::EXIT:
    {
        string user_id = json_value["user_id"].asString();
        string pin = json_value["pin"].asString();
        string room_id = json_value["room_id"].asString();

        cout << "USER:" << user_id << " try to exit ROOM:" << room_id << "." << endl;

        if (!isUserVaild(user_id, pin))
        {
            return sendFailResponse(user_id, "user info not found");
        }
        if (room_map_.count(room_id) == 0)
        {
            return sendFailResponse(user_id, "room not found");
        }

        // success
        room_map_[room_id].members.erase(user_id);
        return sendSuccessResponse(user_id);
        break;
    }
    case ControlType::SEND:
    {
        string user_id = json_value["user_id"].asString();
        string pin = json_value["pin"].asString();
        string room_id = json_value["room_id"].asString();
        string text = json_value["text"].asString();

        cout << "USER:" << user_id << " try to speak in ROOM:" << room_id << "." << endl;

        if (!isUserVaild(user_id, pin))
        {
            return sendFailResponse(user_id, "user info not found");
        }
        if (room_map_.count(room_id) == 0)
        {
            return sendFailResponse(user_id, "room not found");
        }
        if (room_map_[room_id].members.count(user_id) == 0)
        {
            return sendFailResponse(user_id, "user not in room");
        }

        // success
        Json::Value msg;
        msg["ctl"] = static_cast<int>(ControlType::TEXT);
        msg["text"] = text;
        msg["user_id"] = user_id;
        msg["room_id"] = room_id;
        string msg_text = msg.toStyledString();

        for (const string member : room_map_[room_id].members)
        {
            if (!c2s_adapter_map_[member]->sendTextMessage(msg_text))
            {
                // not remove closed user
            }
        }
        return sendSuccessResponse(user_id);
        break;
    }
    case ControlType::GET_ROOMLIST:
    {
        string user_id = json_value["user_id"].asString();
        string pin = json_value["pin"].asString();

        if (!isUserVaild(user_id, pin))
        {
            return sendFailResponse(user_id, "user info not found");
        }
        // success
        Json::Value room_map;
        for (const auto &room_pair : room_map_)
        {
            Json::Value room_info;
            room_info["id"] = room_pair.second.id;
            room_info["owner"] = room_pair.second.owner;
            room_info["is_locked"] = room_pair.second.is_locked;
            room_map[room_pair.first] = room_info;
        }
        return sendSuccessResponse(user_id, room_map);
        break;
    }
    case ControlType::SET_LOCK:
    {
        string user_id = json_value["user_id"].asString();
        string pin = json_value["pin"].asString();
        string room_id = json_value["room_id"].asString();
        bool lock = json_value["lock"].asBool();

        cout << "USER:" << user_id << " try to set ROOM:" << room_id << " lock to " << lock << endl;

        if (!isUserVaild(user_id, pin))
        {
            return sendFailResponse(user_id, "user info not found");
        }
        if (room_map_.count(room_id) == 0)
        {
            return sendFailResponse(user_id, "room not found");
        }
        if (room_map_[room_id].owner != user_id)
        {
            return sendFailResponse(user_id, "user is not owner");
        }

        // success
        room_map_[room_id].is_locked = lock;
        return sendSuccessResponse(user_id);
        break;
    }
    default:
        return sendFailResponse(fd, "unkonwn command");
        break;
    }
}
