Chat room and messaging
=======================

There are different types of chat rooms. Basic chat rooms are simple chat rooms between two participants, they do not require a conference server and are considered as one-to-one chat rooms. Group chat rooms allow more than one peer user and require a conference server. Group chat rooms can also be considered as one-to-one if they have only one peer participant at the time of their creation and if the parameter "enable_one_to_one_chat_room" is set to true, otherwise a basic chat room will be created instead. Secured chat rooms are group chat rooms where LIME X3DH end-to-end encryption is enabled, they can be either group chat rooms or one-to-one chat rooms but not basic chat rooms.

Basic chat rooms
----------------

Basic chat rooms can be created with :cpp:func:`linphone_core_get_chat_room <linphone_core_get_chat_room>` using a peer sip uri.

.. code-block:: c

	LinphoneChatRoom* chat_room = linphone_core_get_chat_room(linphoneCore, "sip:joe@sip.linphone.org");

Once created, messages are sent using :cpp:func:`linphone_chat_room_send_message`.

.. code-block:: c

	linphone_chat_room_send_message(chatRoom, "Hello world");

Incoming message are received through callbacks which can be set after the chat room is instantiated (**LinphoneChatRoomStateInstantiated**).

.. code-block:: c

	<example>

.. seealso:: A more complete tutorial can be found at :ref:`"Chatroom and messaging" <chatroom_code_sample>` source code.

Group chat rooms
----------------

Group chat rooms can be created with :cpp:func:`linphone_core_create_client_group_chat_room <linphone_core_create_client_group_chat_room>`. The third argument is a "fallback" boolean which is taken into account when creating a group chat room with only one peer which does not support group chat. In this case and if this argument is true, a basic chat room will be created instead of not being able to create a group chat room.

.. code-block:: c

	LinphoneChatRoom *chatRoom = linphone_core_create_client_group_chat_room(linphoneCore, "Colleagues", FALSE);

Participants can be invited to the chat room using :cpp:func:`linphone_chat_room_add_participant <linphone_chat_room_add_participant>` or :cpp:func:`linphone_chat_room_add_participants <linphone_chat_room_add_participants>`. Participants can be removed using :cpp:func:`linphone_chat_room_remove_participant <linphone_chat_room_remove_participant>`.

.. code-block:: c

	linphone_chat_room_add_participants(chatRoom, participantsAddressList);
	linphone_chat_room_remove_participant(marieChatRoom, laureParticipant); // remove Laure from Marie's chat room


The list of participants of a chat room can be obtained with :cpp:func:`linphone_chat_room_get_participants <linphone_chat_room_get_participants>`. Note that Marie is not considered as a participant in Marie's chat rooms, one's own participant can be obtained with :cpp:func:`linphone_chat_room_get_me <linphone_chat_room_get_me>`.

.. code-block:: c

	linphone_chat_room_get_participants(chatRoom);
	linphone_chat_room_get_me();

Simple text chat message can be created with :cpp:func:`linphone_chat_room_create_message <linphone_chat_room_create_message>` and sent with :cpp:func:`linphone_chat_message_send <linphone_chat_message_send>`.

.. code-block:: c

	LinphoneChatMessage *message = linphone_chat_room_create_message(marieChatRoom, "Hey!");
	linphone_chat_message_send(message);
	linphone_chat_message_unref(message);

More elaborate chat messages can be built using :cpp:func:`linphone_chat_room_create_empty_message <linphone_chat_room_create_empty_message>` to create an empty message, which can be filled with different contents using :cpp:func:`linphone_chat_message_add_text_content <linphone_chat_message_add_text_content>` and/or :cpp:func:`linphone_chat_message_add_file_content <linphone_chat_message_add_file_content>`.

.. code-block:: c

	LinphoneChatMessage *message = linphone_chat_room_create_empty_message(chatRoom);
	linphone_chat_message_add_text_content(message, content);

Concerning admins, events, history and instant message disposition notifications, more information can be found around the following functions: :cpp:func:`linphone_chat_room_set_participant_admin_status <linphone_chat_room_set_participant_admin_status>`, :cpp:func:`linphone_chat_room_get_history_events <linphone_chat_room_get_history_events>`, :cpp:func:`linphone_chat_room_get_history_range <linphone_chat_room_get_history_range>`, :cpp:func:`linphone_chat_room_mark_as_read <linphone_chat_room_mark_as_read>`.

Secured chat rooms
------------------

LIME X3DH end-to-end encryption for instant messages are enabled in secured chat rooms, also known as encrypted chat rooms, which can be created with :cpp:func:`linphone_core_create_client_group_chat_room_2 <linphone_core_create_client_group_chat_room_2>`. Secured chat rooms and regular chat rooms can coexist, even if they have exactly the same participants.

.. code-block:: c

	LinphoneChatRoom *securedChatRoom = linphone_core_create_client_group_chat_room_2(linphoneCore, "Secured Conversation", FALSE, TRUE);

Encrypted chat rooms only allow encrypted messages and files to transit (except for error IMDNs in case a message was incorrectly decrypted). Encrypted chat rooms have a concept of security level based on LIME X3DH trust level of each participant device in the conference. The current security level of a chat room can be obtained with :cpp:func:`linphone_chat_room_get_security_level <linphone_chat_room_get_security_level>`.

.. code-block:: c

	LinphoneChatRoomSecurityLevel securityLevel = linphone_chat_room_get_security_level(securedChatRoom);

.. seealso:: <point to basic LIME X3DH test and LIME helloworld test>.

.. warning:: LIME X3DH encryption activation at linphone core level requires a server. Make sure the configuration entry `lime/x3dh_server_url` is defined or call :cpp:func:`linphone_core_set_lime_x3dh_server_url()` after core initialisation.
