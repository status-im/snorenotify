# Changes since 0.5.0rc2 #
- Fixed a crash related to a QTimer deleted in the wrong thread.

# Changes since 0.5.0rc1 #
- Cleanedup build system using new Cmake magic
- Dropped KDE stuff
- Dropeed dependencie to boost and cryptopp by switching to a different Growl implementation. [A fork of gntp-send](https://github.com/Snorenotify/SnoreGrowl).


# Changes since 0.5.0beta4 #
- Merged mac osx backend
- Fixed a possible crash, if register was called directly before notify
- Don't register snorenotify with Growl

# Changes since 0.5.0beta3 #
- Only use QTemporaryDir  in qt5

# Changes since 0.5.0beta2 #
- Use  QTemporaryDir for the image cache
- Reduce calls to QFile::exists

# Changes since 0.5.0beta1 #
- Removed plugin caching, it was overkill and caused problems if the plugins where updated.

# Changes since 0.5-alpha1 #
- Better scaling for the snore backend.
- Fixed a crash.

# Changes since 0.4 #
- Support for separated installs for qt4 and qt5.
- Improved logging system.
- Extended doxygen doc.
- Refactoring.
- Added a default qml notification backend.
- Improved plugin loading.
- The Windows 8 backend now supports closing of notifications.
....