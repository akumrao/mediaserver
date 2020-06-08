To debug webrtc


chrome://webrtc-internals

firefox
about:webrtc
about:config

enable logging of webrtc

chrome --enablewebrtc log


To compile webrtc
Download and Install the Chromium depot tools.

https://webrtc.github.io/webrtc-org/native-code/development/prerequisite-sw/

*Linux (Ubuntu/Debian)*
A script is provided for Ubuntu, which is unfortunately only available after your first gclient sync:

./build/install-build-deps.sh


https://webrtc.github.io/webrtc-org/native-code/development/

- mkdir webrtc-checkout
- cd webrtc-checkout
- fetch --nohooks webrtc
- gclient sync



- $ cd src
- $ git checkout -b m84 refs/remotes/branch-heads/4147
- $ gclient sync
- In OSX 10.14.16 this works:
- $ gn gen out/m84 --args='is_debug=false is_component_build=false is_clang=true rtc_include_tests=false rtc_use_h264=true rtc_enable_protobuf=false use_rtti=true mac_deployment_target="10.11" use_custom_libcxx=false'
- In Linux Debian Stretch with GCC 6.3 this works:
- $ gn gen out/m84 --args='is_debug=false is_component_build=false is_clang=false rtc_include_tests=false rtc_use_h264=true rtc_enable_protobuf=false use_rtti=true use_custom_libcxx=false treat_warnings_as_errors=false use_ozone=true'


- Then build it:
- $ ninja -C out/m84


-Then build mediaserver/src/webrtc:



$ cmake . -Bbuild \
  -DLIBWEBRTC_INCLUDE_PATH:PATH=/home/foo/src/webrtc-checkout/src \
  -DLIBWEBRTC_BINARY_PATH:PATH=/home/foo/src/webrtc-checkout/src/out/m84/obj

$ make -C build/




