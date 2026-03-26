#ifndef SLYMAIL_MSG_LIST_H
#define SLYMAIL_MSG_LIST_H

#include "terminal.h"
#include "colors.h"
#include "ui_common.h"
#include "qwk.h"
#include "settings.h"

enum class ConfListResult
{
    Selected,
    Quit,
    OpenFile,
    RemoteSystems,
    Settings,
    Voting,
    SaveRep,
};

enum class MsgListResult
{
    ReadMessage,
    Back,
    Quit,
    NewMessage,
    Settings,
    OpenFile,
    RemoteSystems,
    SaveRep,
};

// Show the conference list
ConfListResult showConferenceList(QwkPacket& packet, int& selectedConf,
                                 const Settings& settings);

// Show the message list for a conference (DDMsgReader-style lightbar)
// Matches DDMsgReader2 screenshot: Msg#, From, To, Subject, Date, Time columns
MsgListResult showMessageList(QwkConference& conf, int& selectedMsg,
                              const Settings& settings,
                              const std::string& bbsName);

#endif // SLYMAIL_MSG_LIST_H
