project('slavery', 'c',	license: 'MIT', version: '0.1',	default_options: ['warning_level=3', 'c_std=c17'])

description = 'Linux user space driver for Logitech MX Master 3 mice.'
authors = ['Simon Allen']

subdir('src')
subdir('doc')

dependencies = [dependency('threads'), dependency('libudev'), dependency('json-c'), dependency('libevdev')]

add_project_arguments('-DPROJECT_NAME="' + meson.project_name() + '"', language: 'c')
add_project_arguments('-DPROJECT_LIBRARY_NAME="lib' + meson.project_name() + '"', language: 'c')
add_project_arguments('-DPROJECT_VERSION="' + meson.project_version() + '"', language: 'c')
add_project_arguments('-DPROJECT_DESC="' + description + '"', language: 'c')

if get_option('buildtype').startswith('debug')
	add_project_arguments('-DDEBUG', language: 'c')
endif

libslavery = library(meson.project_name(), src_libslavery, dependencies: dependencies, install: true)
slavery = executable(meson.project_name(), src_slavery, link_with: libslavery, install: true)
test_config = executable('test_config', 'tests/test_config.c', 
                         link_with: libslavery,
						 include_directories: 'src')
test_monitor = executable('test_monitor', 'tests/test_monitor.c',
                          link_with: libslavery,
						  include_directories: 'src')

pkg = import('pkgconfig')
pkg.generate(libslavery,
             description: description,
			 url: 'https://github.com/garfunkel/slavery')

test('test_config', test_config, workdir: meson.project_source_root() + '/tests', args: 'config.json')
test('test_monitor', test_monitor, workdir: meson.project_source_root() + '/tests')