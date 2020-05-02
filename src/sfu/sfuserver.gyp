{
  'target_defaults': {
    'type': 'executable',
    'dependencies':
    [
      'deps/openssl/openssl.gyp:openssl',
      'deps/libsrtp/libsrtp.gyp:libsrtp',
      'deps/usrsctp/usrsctp.gyp:usrsctp',
      'deps/libsdptransform/sdptransform.gyp:sdptransform',
      'deps/libwebrtc/libwebrtc.gyp:libwebrtc',
      'deps/libwebrtc/deps/abseil-cpp/abseil-cpp.gyp:abseil'
      
    ],
    # TODO: SCTP_DEBUG must be dynamic based on a condition variable in common.gyp.
    # 'defines': [ 'SCTP_DEBUG' ],
    'sources':
    [
      '<!@(ls -1 src/*.cpp)',
      '<!@(ls -1 src/RTCP/*.cpp)',
      '<!@(ls -1 src/sdp/*.cpp)',
      '<!@(ls -1 src/Codecs/*.cpp)',
      '<!@(ls -1 src/SctpDictionaries/*.cpp)',
      '<!@(ls -1 src/Channel/*.cpp)',
      '<!@(ls -1 src/Utils/*.cpp)',
      '<!@(ls -1 src/RtpDictionaries/*.cpp)',
      '<!@(ls -1 ../../src/net/src/*.cpp)',
      '<!@(ls -1 ../../src/json/src/*.cpp)',
      '<!@(ls -1 ../../src/http_parser/*.cpp)', 
      '<!@(ls -1 ../../src/http/src/*.cpp)',
      '<!@(ls -1 ../../src/signal/src/*.cpp)',
      '<!@(ls -1 ../../src/base/src/*.cpp)',
      '<!@(ls -1 ../../src/libuv/src/*.cpp)',
      '<!@(ls -1 ../../src/libuv/src/unix/*.cpp)'
    ],
    'include_dirs':
    [
      'include',
      'deps/libsdptransform/include/',
      '../../src/libuv/include/',
      '../../src/libuv/src/',
      '../../src/signal/include/',
      '../../src/json/include/',
      '../../src/net/include/',
      '../../src/http/include/', 
      '../../src/http_parser/',
      '../../src/base/include/'
    ],
    'conditions':
    [
      # FIPS.
      [ 'openssl_fips != ""', {
        'defines': [ 'BUD_FIPS_ENABLED=1' ]
      }],

      # Endianness.
      [ 'node_byteorder == "big"', {
          # Define Big Endian.
          'defines': [ 'MS_BIG_ENDIAN' ]
        }, {
          # Define Little Endian.
          'defines': [ 'MS_LITTLE_ENDIAN' ]
      }],

      # Platform-specifics.

      [ 'OS == "mac" and sfuserver_asan == "true"', {
        'xcode_settings':
        {
          'OTHER_CFLAGS': [ '-fsanitize=address' ],
          'OTHER_LDFLAGS': [ '-fsanitize=address' ]
        }
      }],

      [ 'OS == "linux"', {
        'defines':
        [
          '_POSIX_C_SOURCE=200112',
          '_GNU_SOURCE'
        ]
      }],

      [ 'OS == "linux" and sfuserver_asan == "true"', {
        'cflags': [ '-fsanitize=address' ],
        'ldflags': [ '-fsanitize=address' ]
      }],

      [ 'OS in "linux freebsd"', {
        'ldflags': [ '-Wl,--whole-archive <(libopenssl) -Wl,--no-whole-archive' ]
      }],

      [ 'OS in "freebsd"', {
        'ldflags': [ '-Wl,--export-dynamic' ]
      }],

      [ 'OS == "win"', {
        'dependencies': [ 'deps/getopt/getopt.gyp:getopt' ],

        # Handle multi files with same name.
        # https://stackoverflow.com/a/22936230/2085408
        # https://docs.microsoft.com/en-us/dotnet/api/microsoft.visualstudio.vcprojectengine.vcclcompilertool.objectfile?view=visualstudiosdk-2017#Microsoft_VisualStudio_VCProjectEngine_VCCLCompilerTool_ObjectFile
        'msvs_settings': {
          'VCCLCompilerTool': { 'ObjectFile': ['$(IntDir)\%(RelativeDir)\%(Filename).obj'], },
        },

        # Output Directory setting for msvc.
        # https://github.com/nodejs/node-gyp/issues/1242#issuecomment-310921441
        'msvs_configuration_attributes': {
          'OutputDirectory': '$(SolutionDir)\\out\\$(Configuration)\\'
        }
      }],

      [ 'OS != "win"', {
        'cflags': [ '-std=c++11', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-implicit-fallthrough' ]
      }],

      [ 'OS == "mac"', {
        'xcode_settings':
        {
          'WARNING_CFLAGS': [ '-Wall', '-Wextra', '-Wno-unused-parameter' ],
          'OTHER_CPLUSPLUSFLAGS' : [ '-std=c++11' ]
        }
      }],

      # Dependency-specifics.

      [ 'sctp_debug == "true"', {
        'defines': [ 'SCTP_DEBUG' ]
      }]
    ]
  },
  'targets':
  [
    {
      'target_name': 'sfuserver',
      'sources':
      [
        # C++ source files.
        'sfuserver/main.cpp'
      ],
      "libraries": 
      [
        '-lpthread',
        '-lm',
        '-ldl'
      ]
    }
  ]
}
