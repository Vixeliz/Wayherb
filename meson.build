project(
  'wayherb',
  'c',
  license: 'MIT',
)

wayherb_inc = include_directories('include')

cc = meson.get_compiler('c')

cairo = dependency('cairo')
threads = dependency('threads')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
wayland_protos = dependency('wayland-protocols')

subdir('lib')

wayherb_files = [
  'src/util.c',
  'src/wayherb.c',
  'src/wayland.c',
]

executable(
  'wayherb',
  files(wayherb_files),
  dependencies: [
    cairo,
    client_protos,
    threads,
    wayland_client,
    wayland_cursor,
  ],
  include_directories: [wayherb_inc],
  install: true,
)
