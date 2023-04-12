project('rscode', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])


ecc = static_library('ecc', ['crcgen.c', 'galois.c', 'rs.c', 'berlekamp.c'])
ecc_deps = declare_dependency(
  include_directories: include_directories('.'),
  link_with: ecc)

executable('example', 'example.c',
           dependencies : ecc_deps,
           install : true)