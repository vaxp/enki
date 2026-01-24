# ============================================================
# ENKI Quantum Calculator — Real Application Build
# ============================================================

calculator_sources = files(
  'src/calculator_engine.cpp',
  'src/main.cpp',
)

enki_calculator_exe = executable('enki_calculator',
  sources: calculator_sources,
  include_directories: include_directories('include'),
  dependencies: [enki_dep],
  install: true,
)
