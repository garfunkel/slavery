project('libslavery', 'c',
	version: '0.1',
	default_options: ['warning_level=3', 'c_std=c17'])

libslavery = library('slavery', 'libslavery.c', dependencies: dependency('libudev'))

executable('slavery',
	'slavery.c',
	link_with: libslavery,
	install: true)
