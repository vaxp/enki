project('enki_example_app', 'cpp',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=2',
    'optimization=3'
  ]
)

enki_dep = dependency('enki', required: false)

if not enki_dep.found()
  enki_proj = subproject('enki')
  enki_dep = enki_proj.get_variable('enki_dep')
endif

executable('enki_example_app',
  sources: [
    'counter_cubit.cpp'
  ],
  dependencies: [
    enki_dep
  ],
  install: true
)
