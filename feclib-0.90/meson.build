project('feclib', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

cc = meson.get_compiler('c')
fec_deps = []
if host_machine.system() == 'windows'
  add_global_arguments('-DWIN32', language : 'c')
  fec_deps += [cc.find_library('ws2_32')]
endif


fec = static_library('fec', ['fec.c', 'fec.h'])
fec_deps += declare_dependency(
  include_directories: include_directories('.'),
  link_with: fec)

executable('fecrecv', 'fecrecv.c',
           dependencies : fec_deps)

executable('fecsend', 'fecsend.c',
           dependencies : fec_deps)

executable('fectest', 'fectest.c',
           dependencies : fec_deps)