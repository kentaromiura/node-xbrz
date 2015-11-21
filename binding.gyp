{
  "targets": [
    {
      "target_name": "xbrz",
      "sources": [ "xbrzm.cc", "xbrz/xbrz.cpp" ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'MACOSX_DEPLOYMENT_TARGET': '10.7',
            'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11','-stdlib=libc++'],
            'OTHER_LDFLAGS': ['-stdlib=libc++']
          }
        }]
      ],
      'libraries':[
        "-lpng"
      ]
    }
  ]
}
