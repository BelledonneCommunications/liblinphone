# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

# Preamble

This changelog file was started on October 2019. Previous changes were more or less tracked in the *NEWS* file.

## [Unreleased]


## [4.4.0] 2020-06-09
    
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

