# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

# Preamble

This changelog file was started on October 2019. Previous changes were more or less tracked in the *NEWS* file.

## [Unreleased]

## [4.5.2] 2021-04-14

### Added
- CoreService class for Android can make the device vibrate while incoming call is ringing.

## [4.5.0] 2021-03-29

### Added
- Audio conference API improved: distribution of participant list over RFC4575 SUBSCRIBE/NOTIFY.
- Automatic handling of some mobile OS behaviours
  * enterForeground() and enterBackground() automatically called (iOS and Android).
  * auto acquire and release of audio focus for Android.
  * Core.iterate() is being called automatically internally for Android, it is no longer needed to create a timer in the application to call it.
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

