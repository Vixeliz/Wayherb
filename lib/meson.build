wl_protocol_dir = wayland_protos.get_pkgconfig_variable('pkgdatadir')

wayland_scanner = find_program('wayland-scanner')

wayland_scanner_code = generator(
  wayland_scanner,
  output: '@BASENAME@.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@'],
)

wayland_scanner_client = generator(
  wayland_scanner,
  output: '@BASENAME@.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@'],
)

client_protocols = [
  [wl_protocol_dir, 'stable/xdg-shell/xdg-shell.xml'],
  ['wlr-layer-shell-unstable-v1.xml'],
]

client_protos_src = []
client_protos_headers = []
foreach p : client_protocols
  xml = join_paths(p)
  client_protos_src += wayland_scanner_code.process(xml)
  client_protos_headers += wayland_scanner_client.process(xml)
endforeach

client_protos = declare_dependency(
  sources: [client_protos_headers, client_protos_src],
)
