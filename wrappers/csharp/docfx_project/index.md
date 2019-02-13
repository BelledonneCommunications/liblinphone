<!--
Copyright (c) 2010-2022 Belledonne Communications SARL.

This file is part of Liblinphone 
(see https://gitlab.linphone.org/BC/public/liblinphone).

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
-->
# Welcome to liblinphone C# API reference documentation

Liblinphone is a high-level open source library that integrates all the SIP voice/video and instant messaging features into a single easy-to-use API. This is the VoIP SDK engine on which Linphone applications are based.

Liblinphone combines our media processing and streaming toolkit (Mediastreamer2) with our user-agent library for SIP signaling (belle-sip).

Liblinphone is distributed under GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html). Please understand the licencing details before using it!
For any use of this library beyond the rights granted to you by the GPLv3 license, please [contact Belledonne Communications](https://www.linphone.org/contact).

## CSharp tutorial for liblinphone

You can find a step by step tutorial to use liblinphone in C# [here](https://gitlab.linphone.org/BC/public/tutorials)

## Other supported languages

 Liblinphone has support for a variety of languages, each one has its own reference documentation:

 - C (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/c)
 - C++ (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/c++)
 - Swift (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/swift)
 - Java (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/java)
 - C# (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/cs)
 - Python (https://download.linphone.org/@LINPHONESDK_STATE@/docs/liblinphone/@STRIPPED_LINPHONE_VERSION@/python)

## See also
http://www.linphone.org

# Quick tour of liblinphone's features

## Introduction

Liblinphone's has a consistent object-oriented design.
Root objects must be constructed by the @Linphone.Factory class. No 'new' operator invocation is permitted.
Liblinphone is using SIP as signaling protocol, which actually comprises a huge set of RFCs to cover various aspects of communications. Some terminology of the API is directly inherited from
SIP specifications, that's why having some knowledge of the protocol is recommended for a better understanding of this documentation.

## Initializing the engine
A typical liblinphone application has to first instanciate a @Linphone.Core object using the @Linphone.Factory. The core object represents the liblinphone engine, from which call, conferences, instant messages can be sent or received.
For events to be reported and engine to schedule its tasks, the application must call @Linphone.Core.Iterate at regular interval, typically from a 20ms timer.
In most case, a SIP account has to be provisionned so that SIP registration can take place onto a SIP server. This task is designated to the Account class.
An Account can be created using @Linphone.Core.CreateAccount(Linphone.AccountParams), based on parameters created with @Linphone.Core.CreateAccountParams.
Then, account can be added to the core for usage using @Linphone.Core.AddAccount(Linphone.Account).
Application usually need to get informed of events occuring in the lifetime of the engine, which is done through listeners that the applications can override.
An important listener interface is the @Linphone.CoreListener, that application should override and and create, and then assign into their
@Linphone.Core object through @Linphone.Core.Listener property.

## Making calls
Applications can place outgoing calls using @Linphone.Core.Invite(System.String) or @Linphone.Core.InviteAddressWithParams(Linphone.Address,Linphone.CallParams).
The @Linphone.CallParams class represents parameters for the calls, such as enabling video, requiring a specific @Linphone.MediaEncryption.
The @Linphone.CallListener interface provides application way to get inform of the progress of the call, represented by the @Linphone.CallState enum.
Incoming calls are notified through the @Linphone.CoreListener interface, and can later be accepted using @Linphone.Call.Accept .
Calls can be terminated or aborted at any time using @Linphone.Call.Terminate .

## Instant messaging

The @Linphone.ChatRoom class represents a text conversation. The @Linphone.Core object provides persistancy for all conversations, ie it stores all received and sent messages.
The list of conversation can be retrieved using @Linphone.Core.ChatRooms.
To create a new conversation, use @Linphone.Core.CreateChatRoom(Linphone.ChatRoomParams,System.String,System.Collections.Generic.IEnumerable{Linphone.Address}). @Linphone.ChatRoomParams provide a way to specify which kind of chatroom is to be created: for group,
for one-ton-one conversation, with end-to end encryption for example.
To send a message, first create the @Linphone.ChatMessage with @Linphone.ChatRoom.CreateMessageFromUtf8(System.String), then send it with @Linphone.ChatMessage.Send .
A @Linphone.ChatMessage reports its progress through the @Linphone.ChatMessageListener interface.
ChatRooms are automatically created by the @Linphone.Core when receiving a message that starts a new conversation, and notified through the @Linphone.CoreListener interface.

## Presence

Applications can submit presence information through the Core's @Linphone.Core.PresenceModel property. The @Linphone.PresenceModel class represents the presence information, which is submitted to a presence server.
Symmetrically, applications can subscribe to the presence server to get notified of the presence status of a contact list.
This is to be done thanks to the @Linphone.FriendList and @Linphone.Friend classes.

