Chat room and messaging
=======================
Exchanging text messages
------------------------

There are different types of chat rooms. Basic chat rooms are simple chat rooms between two participants and do not require a conference server. Basic chat rooms can be created with :cpp:func:`linphone_core_get_chat_room <linphone_core_get_chat_room>` using a peer sip uri.

.. code-block:: c

	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(lc,"sip:joe@sip.linphone.org");

Once created, messages are sent using :cpp:func:`linphone_chat_room_send_message`.

.. code-block:: c

	linphone_chat_room_send_message(chat_room,"Hello world"); /*sending message*/

Incoming message are received from call back LinphoneCoreVTable.text_received

.. code-block:: c

	void text_received(LinphoneCore *lc, LinphoneChatRoom *room, const LinphoneAddress *from, const char *message) {
		printf(" Message [%s] received from [%s] \n",message,linphone_address_as_string (from));
	}

.. seealso:: A more complete tutorial can be found at :ref:`"Chatroom and messaging" <chatroom_code_sample>` source code.

Group chat rooms allow more than one peer user and require a conference server. Group chat rooms can be created with :cpp:func:`linphone_core_create_client_group_chat_room <linphone_core_create_client_group_chat_room>`.

.. code-block:: c

	LinphoneChatRoom *chatRoom = linphone_core_create_client_group_chat_room(lc, "Colleagues", FALSE);

Participants can be invited to the chat room using :cpp:func:`linphone_chat_room_add_participants <linphone_chat_room_add_participants>`.

.. code-block:: c

	linphone_chat_room_add_participants(chatRoom, participantsAddressList);

LIME X3DH end-to-end encryption for instant messages are enabled in "Encrypted" chat rooms, which can be created with :cpp:func:`linphone_core_create_client_group_chat_room_2 <linphone_core_create_client_group_chat_room_2>`.

.. code-block:: c

	LinphoneChatRoom *securedChatRoom = linphone_core_create_client_group_chat_room_2(lc, "Secured Conversation", FALSE, TRUE);

.. seealso:: A more complete tutorial can be found at :cpp:func:`helloworld_basic_test` source code in lime submodule.

Encrypted chat rooms only allow encrypted messages and files to transit (except for error IMDNs in case a message was incorrectly decrypted). Encrypted chat rooms have a concept of security level based on LIME X3DH trust level of each participant device in the conference. The current security level of a chat room can be obtained with :cpp:func:`linphone_chat_room_get_security_level <linphone_chat_room_get_security_level>`.

.. code-block:: c

	LinphoneChatRoomSecurityLevel securityLevel = linphone_chat_room_get_security_level(securedChatRoom);