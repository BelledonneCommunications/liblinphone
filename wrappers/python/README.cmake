<!--
Copyright (c) 2010-2024 Belledonne Communications SARL.

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

# Python API reference documentation

Liblinphone is a high-level open source library that integrates all the SIP voice/video and instant messaging features into a single easy-to-use API. This is the VoIP SDK engine on which Linphone applications are based.

Liblinphone combines our media processing and streaming toolkit (Mediastreamer2) with our user-agent library for SIP signaling (belle-sip).

Liblinphone is distributed under GPLv3 (https://www.gnu.org/licenses/gpl-3.0.html). Please understand the licencing details before using it!
For any use of this library beyond the rights granted to you by the GPLv3 license, please [contact Belledonne Communications](https://www.linphone.org/contact).

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

# Getting started

Here's how to import our module and print the liblinphone version that you are using:
```python
import linphone
print(linphone.Core.get_version())
```

# Quick tour of liblinphone's features

## Introduction

Liblinphone's has a consistent object-oriented design.
Root objects must be constructed by the `Factory` class. No 'new' operator invocation is permitted.
Liblinphone is using SIP as signaling protocol, which actually comprises a huge set of RFCs to cover various aspects of communications. Some terminology of the API is directly inherited from
SIP specifications, that's why having some knowledge of the protocol is recommended for a better understanding of this documentation.

## Initializing the engine
A typical liblinphone application has to first instanciate a `Core` object using the `Factory`. The core object represents the liblinphone engine, from which call, conferences, instant messages can be sent or received.
For events to be reported and engine to schedule its tasks, the application must call `Core.iterate()` at regular interval, typically from a 20ms timer.
In most case, a SIP account has to be provisionned so that SIP registration can take place onto a SIP server. This task is designated to the Account class.
An Account can be created using `Core.create_account()`, based on parameters created with `Core.create_account_params()`.
Then, account can be added to the core for usage using `Core.add_account()`.
Application usually need to get informed of events occuring in the lifetime of the engine, which is done through listeners that the applications can override.
An important listener interface is the `CoreListener`, that application should create, and then assign into their
`Core` object through `Core.add_listener()` method.

## Making calls
Applications can place outgoing calls using `Core.invite()` or `Core.invite_address_with_params()`.
The `CallParams` class represents parameters for the calls, such as enabling video, requiring a specific `MediaEncryption`.
The `CallListener` interface provides application way to get inform of the progress of the call, represented by the `CallState` enum.
Incoming calls are notified through the `CoreListener` interface, and can later be accepted using `Call.accept()` .
Calls can be terminated or aborted at any time using `Call.terminate()` .

## Instant messaging

The `ChatRoom` class represents a text conversation. The `Core` object provides persistancy for all conversations, ie it stores all received and sent messages.
The list of conversation can be retrieved using `Core.chat_rooms` property.
To create a new conversation, use `Core.create_chat_room()`. `ChatRoomParams` provide a way to specify which kind of chatroom is to be created: for group,
for one-ton-one conversation, with end-to end encryption for example.
To send a message, first create the `ChatMessage` with `ChatRoom.create_message_from_utf8()`, then send it with `ChatMessage.send()` .
A `ChatMessage` reports its progress through the `ChatMessageListener` interface.
ChatRooms are automatically created by the `Core` when receiving a message that starts a new conversation, and notified through the `CoreListener` interface.

## Presence

Applications can submit presence information through the Core's `Core.presence_model` property. The `PresenceModel` class represents the presence information, which is submitted to a presence server.
Symmetrically, applications can subscribe to the presence server to get notified of the presence status of a contact list.
This is to be done thanks to the `FriendList` and `Friend` classes.