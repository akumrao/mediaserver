{
  'targets': 
  [
    {
      'target_name': 'sdptransform',
      'type': 'static_library',
      'sources':
      [
        'include/sdptransform.hpp',
        'src/grammar.cpp',
        'src/parser.cpp',
        'src/writer.cpp'
      ],
      'include_dirs': 
      [
        'include/'
      ],
      'direct_dependent_settings':
      {
        'include_dirs': ['src/']
      },
    }
  ]
}
