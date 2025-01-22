# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

# Preamble

This changelog file was started on October 2019. Previous changes were more or less tracked in the *NEWS* file.

## Unreleased

### Changed
- Acoustic Echo Canceller upgraded to the AEC3 from a recent version of WebRTC.
- The delay measured by the Echo Canceller Calibrator is not used in mswebrtcaec.
- Voice Activity Detection updated to a recent version of WebRTC with minor changes.

### Removed
- AECM for mobile echo cancellation is removed and replaced by the new AEC.
- removed ISAC and iLBC audio codecs.

## [5.4.0] 2025-03-11

### Added
- Forward Error Correction to video stream, based on FlexFEC RFC8627
- Java wrapper for Linux and Windows (see *java-sdk-linux* and *java-sdk-windows* linphone-sdk cmake presets)
- Python wrapper for Linux (use -DENABLE_PYTHON_WRAPPER=ON)
- Screensharing.
- Creation of conferences through CCMP protocol (RFC6503) via ConferenceScheduler object.
- VCARD version 3 compatibility (in addition to version 4).
- Bearer authentication for SIP and http utilities (remote provisioning, file transfer...).
- Post-Quantum IM end-to-end encryption (see lime/CHANGELOG.md)
- Added LinphoneAccountManagerServices class to encapsulate various account management features provided by Flexisip Account Manager
- LinphoneMagicSearch can now perform contact searches through CardDav protocol.
- Search text in IM message feature.
- IM capability within video conferences
- Message Waiting Indication client support (RFC3842): see LinphoneMessageWaitingIndication object.
- Add Simple Media File Format support for recording of calls (house-made audio/video file container that supports opus, AV1, H264, H265, VP8 codecs).
- linphone_participant_device_get_thumbnail_stream_capability() returns the direction of the thumbnail stream of a participant device.
- new linphone_video_activation_policy_set_automatically_accept_direction() to help apps handle asymmetric video activation during calls.
- LinphoneFriendDevice object, to present user's device and associated security levels.
- Automatic retry of pending outgoing IMs in case of timeout or network disconnection.
- BETA features:
  - SFU-based audio and video conferencing
  - End to end encryption for audio/video conferences, using double SRTP encryption RFC8723 and RFC8870 (EKT)

### Changed
- Unifies ChatRoom and Conferences internal classes
- Allow IMDNs to be sent only to message sender (see linphone_core_set_imdn_to_everybody_threshold() )
- Echo Canceller Calibrator improvements
- LinphoneRecorderFileFormat enum has been renamed into LinphoneMediaFileFormat.
- Participant devices now deal separately with the main video stream and the thumbnail stream
  linphone_participant_device_get_ssrc() for the video stream now returns the SSRC of the main stream. The SSRC of the thumbnail stream is now returned by linphone_participant_device_get_thumbnail_ssrc() 
  linphone_participant_device_get_stream_availability() for the video stream now returns the overall video availability of a participant device. Function linphone_participant_device_get_thumbnail_stream_availability() now return whether a participant is sending its video stream or not
- The participant device callbacks follows changes made on the participant device API
  LinphoneParticipantDeviceCbsStreamAvailabilityChangedCb for the video stream is trigged by an change on the overall video availability. LinphoneParticipantDeviceCbsThumbnailStreamAvailabilityChangedCb is now called when a participant device toggle its camera setting
- linphone_core_get_video_activation_policy() now returns a const object. Use linphone_video_activation_policy_clone() or create a new one
  if you want to make changes before calling linphone_core_set_video_activation_policy().
- LinphoneFriendList revision field is now a string instead of an int. If there was a revision value > 0, it is kept but as string, otherwise newly created friend list will have a revision set to empty string.
- Improve ZRTP security and alert against security level regressions during calls.
- Improve reliability of push notification handling on Android (avoid process suspension using wake locks)
- CPU usage optimisations while using a large number of Account objects (typically in server applications)
- internal refactoring of C code into C++.
- AccountCreator object is deprecated (use LinphoneAccountManagerServices as a replacement)
- Allow LinphoneCoreCbs' s authentication_requested() callack to be invoked during remote provisioning.
- Disable peer to peer presence by default (linphone_friend_subscribes_enabled() is FALSE, and linphone_friend_get_inc_subscribe_policy() returns LinphoneSPDeny).

### Fixed
- Added _utf8() getter and setters for subject of conference and chatrooms.
  Avoid useless conversions when the utf8 variants of the getter/setters are used.
- t= lines in the SDP no longer changed in the answer
- DNS resolution of stun server for ICE is now made asynchronously at the beginning of calls.
- UTF-8 issues with C# wrapper

### Removed
- Removed old & deprecated VideoPolicy object.
- ISAC and iLBC audio codecs are deprecated and will be removed in a future release.


## [5.3.55] 2024-05-31

### Fixed
- Fix a design issue complicating audio route selection on iOS. This version introcuces for iOS
  a new LinphoneAudioDevice instance with a specific "follows system routing" capability
  (see linphone_audio_device_get_follows_system_routing_policy()).
  This LinphoneAudioDevice becomes the default one when upgrading, and simply follows the system routing policy
  when handling calls. Indeed, it is not possible to know prior to a call whether a bluetooth device is available, so 
  the best approach is to let the system device.
  Application can of course later override the audio route by setting other LinphoneAudioDevice representing speaker or bluetooth outputs.

## [5.3.0] 2023-12-18

### Added
- Added notion of roles for conferences, so that there can be speakers and listeners.
- Reactions with IM conversations
- New video codec policy, so that hardware-accelerated codecs are prioritized in offer/answer. See LinphoneCodecPriorityPolicyAuto .
- AV-1 codec
- New LinphoneAlert object used to report various QoS alerts during calls.
- SRTP AEAD_AES_128_GCM and AEAD_AES_256_GCM crypto suites.

### Changed
- Enum relocations dictionnary is now automatically computed, which fixes some enum that were not
  scoped where there have to be. For example, in Swift linphonesw.ConferenceSchedulerState becomes linphonesw.ConferenceScheduler.State.
  This is an API change for C++, Swift and Java, requiring application code to be adapated.
- TLS Client certificate request authentication callback removed (due to mbedtls update).
  Application using TLS client certificate must provide it before any TLS connexion needing it.
- Refactoring of LinphoneAddress object implementation, leading to greater internal simplicity and performance.
- C to C++ internal refactoring for LinphoneEvent, LinphonePayloadType, LinphoneContent

### Fixed
- Documentation issues in C++, Swift and C# wrappers
- Memory leaks

### Removed
- SRTP AES_CM_128_HMAC_SHA1_32 and AES_CM_256_HMAC_SHA1_32 crypto suites, considered unsecure because of SHA1.


## [5.2.0] 2022-11-14

### Added
- Video conferencing feature with "mosaic" and "active speaker + thumbnails" layouts. This feature requires a
  compatible SIP video conferencing server such as the Flexisip conference server. Only VP8 codec is supported for now.
- Added conferencing scheduling API (LinphoneConferenceScheduler), generating ICS invitations sent throug IM.
- Support for post-quantum robust encryption for ZRTP calls by combining Cristals-Kyber algorithm with classic (EC)DH.
- Rtp bundle can be enabled per LinphoneAccount, superseeding the setting at LinphoneCore level.
- New APIs on Friend object to be able to set more info such as a Picture, Organization, Native ID & Starred
- QRCode image generation.

### Changed
- Licence becomes AGPLv3.
- Auto schedule of core.iterate() method now uses a higher delay for timer if app is in background
- Foreground & background delays for core.iterate() scheduling can be configured in linphonerc & using API
- Optimisations for group chat with LIME (end to end encryption).

### Fixed
- Chatrooms erroneously left in case of aborted INVITE transaction (bad network conditions)
- RTP bundle mode performance.
- Improve overall robustness of notification of chatroom events.

## [5.1.0] 2022-02-14

### Added
- LinphoneRecorder API added to record voice messages, that can later be sent in a LinphoneChatMessage.
- A recommended video definitions list that filter the supported video definitions list.
- 1080p is added to the list of supported definitions for Android and iOS targets.
- Android device rotation is now handled by PlatformHelper, apps no longer need to do it.
- LDAP connectivity into LinphoneMagicSearch API (to lookup contacts by number or name).

### Changed
- Java wrapper no longer catches app exceptions that happen in listeners.
- linphone_core_enable_mic() is changed to be persistent accross calls and conference. It no longer
  resets after each call or conference. An AND operation with enablement given by linphone_call_set_microphone_muted() and 
  linphone_conference_set_microphone_muted() is made to determine the actual microphone state.

### Security fixes
- To protect against "SIP digest leak", MD5 and digestion without qop=auth can be disabled by configuration
  See linphone_core_set_digest_authentication_policy() in reference documentation for more details.
  Alternatively the following properties can be added in linphonerc configuration file:
    [digest_authentication_policy]
    allow_md5=0
    allow_no_qop=0
  To preserve maximum interoperability with available SIP services, default values for both options are 1 (true).
  Using a robust password is anyway highly recommended to avoid brute force attacks.


## [5.0.0] 2021-07-08

### Added
- Native support of Windows 10 UWP platform
- CoreService class for Android can make the device vibrate while incoming call is ringing.
- Support of Capability Negociation framework - RFC5939 - limited to media encryption choice (None, SRTP, DTLS-SRTP, ZRTP)
- Automatic handling of push notifications for iOS and Android. 
  * A new state LinphoneCallPushIncomingReceived is added to the LinphoneCall's state machine.
  * New LinphonePushNotificationConfig object added to the API.
- New API to manage SIP accounts: LinphoneAccount and LinphoneAccountParams, replacing LinphoneProxyConfig which is now deprecated.
  LinphoneProxyConfig remains fully usable for backward compatibility with previous version.
- New implementation of LinphoneAccountCreator relying on http REST API. Previous implementation based on XML-RPC remains the default one,
  but will be deprecated in a future release.
- Added LDAP contact provider API integrated with LinphoneMagicSearch (desktop platforms ONLY)
- Added asynchronous API to the LinphoneMagicSearch API (for contact searching).

### Changed
- Java wrapper now creates a separated managed object for const native pointers and print an error log when trying to use a non-const method on it.
- Java & C# wrapper now takes a strong reference on listeners, allowing to use inner classes & lambda expressions even without keeping parent object around.
- Deprecated methods for more than two years have been removed from wrappers (e.g. every setListener() on some objects).
- Automatic BYEing of orphan dialogs - this task no longer needs to be done by the proxy, conforming to RFC3261.
- Generation of C# API documentation now with DocFX.

### Removed
- Legacy Java wrapper for Android

### Fixed
- Erroneous ICE ufrag and passwd parameters sent in reINVITE while ICE was refused previously.
- Swift and C# wrappers corner-case usage issues.
- See git log for full list of other minor bugfixes.


## [4.5.0] 2021-03-29

### Added
- Audio conference API improved: distribution of participant list over RFC4575 SUBSCRIBE/NOTIFY.
- Automatic handling of some mobile OS behaviours
  * enterForeground() and enterBackground() automatically called (iOS and Android).
  * auto acquire and release of audio focus for Android.
  * Core.iterate() is being called automatically internally for Android & iOS, it is no longer needed to create a timer in the application to call it.
- New audio routes API to choose which device use to capture & play audio (Android & iOS). The application can manage
  audio route changes (bluetooth, speaker, earpiece) throug liblinphone's API.
- Added API to play user's ringtone instead of default ringtone for Android.
- Added callback to notify a message is about to be sent.
- iOS: added linphone_core_configure_audio_session() to be called when used with Callkit
  see specific documentation here: https://wiki.linphone.org/xwiki/wiki/public/view/Lib/Getting%20started/iOS/#HCallKitintegration
- client-based video conference in active speaker switching mode (beta feature).

### Changed
- Warning: some function parameters have been renamed for consistency, which modified the Swift API (where parameter names are part of the ABI).
  As a result, adjustements in applications are expected when migrating a swift app based on liblinphone from 4.4 to 4.5.
- Improved Android network manager.
- To make it consistent with other liblinphone's object, linphone_core_create_subscribe(), linphone_core_create_subscribe2(),
  linphone_core_create_publish() now give ownership of the returned LinphoneEvent, which means that it is no longer need to call
  linphone_event_ref() after calling these functions. As a consequence, an application not using linphone_event_ref() should now
  use linphone_event_unref() when the LinphoneEvent is no longer used, otherwise it will create a memory leak.
- Real time text related function linphone_chat_message_get_char() now will always return the new line character,
  which wasn't the case before if the get_char() was done after the composing callback was triggered for this character.
- linphone_core_interpret_url() will unescape characters first if possible if only a username is given as input parameter.
- linphone_chat_message_cancel_file_transfer() no longer deletes the file for outgoing messages.
- magic search result created from filter now applies the international prefix of the default proxy config if possible.
- To improve performance file transfer progress callback will be at most notified 100 times.
- Deprecate linphone_core_audio_route_changed() that was introduced in 4.4, to fix audio issues
  when switching audio to some low sample rate Bluetooth devices. It is now handled internally.

### Fixed
- Internal refactoring of management of locally played tones, in order to fix race conditions.
- Magic search bar not looking in all friends lists.
- Error IMDN in LIME chat rooms not properly sent.
- Chat message lost during attachment auto download if Core stopped during the process.
- Windows tests.
- Name of MediaCodec encoder and decoder filters in H264Helper Java class.
- Both FileContent and FileTransferContent being present in linphone_chat_message_get_contents() list until upload is finished.

## [4.4.0] 2020-06-16
    
### Added
- Simplified integration with CallKit, see https://wiki.linphone.org/xwiki/wiki/public/view/Lib/Getting%20started/iOS/#HCallKitIntegration
- Specific additions to take into account new iOS 13 constraints, see https://wiki.linphone.org/xwiki/wiki/public/view/Lib/Getting%20started/iOS/iOS13%20Migration%20guide/
  * compatibility for inclusion within an app extension
  * helper functions to create a LinphoneCore that can be shared between an app and its extension
  * helper functions to process remote push notifications announcing IM messages 
- RTP bundle mode feature according to https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-54, providing
  increased interoperability with WebRTC.
- TURN over TCP or TLS
- Ephemeral IM - let user program automatic destruction of IM messages.
- SIP Session-Timers (RFC4028)
- New method linphone_core_stop_async(), to ensure clean shutdown can be done without blocking main thread.

### Changed
- Big internal refactoring of how streams are managed within offer/answer exchanges, in order to facilitate bundle mode
  implementation, as well as development of new features.
- ICE now uses all IP addresses detected on the host (except IPv6 temporary ones)
- Better handling of parameter changes in streams during the session, which avoids unecessary restarts.
- Do not notify phone number being too short in account manager anymore, our dial plan isn't precise enough to garanty phone number is invalid in this case
- Swift wrapper is no longer compiled due to its limited binary compatibility. It is instead exported as source code to be compiled with
  the application making use of it.

### Fixed
- Internal refactoring of management of locally played tones, in order to fix race conditions.
- Issues when handling SDP offer with only text stream.
- Random file transfer issues when used with LIME.


## [4.3.0] - 2019-10-14

### Added
- New cmake options to make "small" builds of liblinphone, by excluding adavanced IM and DB storage.

### Changed
- Optimisations in chatrooms loading from Sqlite DB, improving startup time.
- License changed to GNU GPLv3.

