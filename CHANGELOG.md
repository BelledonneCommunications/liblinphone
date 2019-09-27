# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

# Preamble

This changelog file was started on October 2019. Previous changes were more or less tracked in the *NEWS* file.

## [Unreleased]
    
### Added
- RTP bundle mode feature according to https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-54 .

### Changed
- Big internal refactoring of how streams are managed within offer/answer exchanges.
- ICE now uses all IP addresses detected on the host.
- Better handling of parameter changes in streams during the session, which avoids unecessary restarts.

### Fixed
- Internal refactoring of management of locally played tones, in order to fix race conditions.


## [4.3.0] - 2019-10-14

### Added
- New cmake options to make "small" builds of liblinphone, by excluding adavanced IM and DB storage.

### Changed
- Optimisations in chatrooms loading from Sqlite DB, improving startup time.
- License changed to GNU GPLv3.

