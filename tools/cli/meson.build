# ============================================================
# ENKI Native CLI Tool — Standalone Project
# ============================================================

project('enki-cli', 'cpp',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=2',
    'b_ndebug=if-release'
  ]
)

cli_sources = files(
  'src/main.cpp',
  'src/core/process.cpp',
  'src/commands/doctor.cpp',
  'src/commands/devices.cpp',
  'src/commands/create.cpp',
  'src/commands/run.cpp',
  'src/commands/clean.cpp',
)

cli_inc = include_directories('src')

executable('enki',
  sources: cli_sources,
  include_directories: [cli_inc],
  install: true,
)
