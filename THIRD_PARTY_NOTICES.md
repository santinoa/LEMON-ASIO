# Third-party notices

The integrated driver is an adaptation of KoordASIO 2.3.1, which is derived from FlexASIO. The upstream source is pinned as a Git submodule in third_party/KoordASIO; its license and copyright notices are preserved there.

The upstream FlexASIO and related helper projects include MIT-licensed source by Etienne Dechamps. Keep their original copyright and permission notices with those source files.

The driver build uses Steinberg ASIO SDK 2.3.4 under the GPL version 3 option. Its SDK license explicitly offers GPLv3 as an alternative to Steinberg's proprietary license. This project selects GPLv3 and does not redistribute Steinberg's SDK as a separate developer toolkit. The build downloads the SDK for compilation. The official SDK licensing text and usage guidelines are available from Steinberg's ASIO SDK distribution.

PortAudio, Qt, tiny TOML, and other upstream dependencies retain their own licenses and notices. Their source and license metadata are included through the pinned upstream repository and its recursive submodules.

The LEMON Audio branding and integration changes in this repository are offered under GPL-3.0-only. The original prototype's MIT notice is preserved separately.
