# ============================================================
# ENKI Native CLI Tool Build Target
# ============================================================

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
