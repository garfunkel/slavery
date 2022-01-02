doxygen = find_program('doxygen', required: false)
year = run_command('date', '+%Y').stdout().strip()

if doxygen.found()
	run_target('doc',
	           command: [doxygen, '@CURRENT_SOURCE_DIR@/Doxyfile'],
			   env: {'PROJECT_NAME': meson.project_name(),
			         'PROJECT_BRIEF': description,
					 'PROJECT_VERSION': meson.project_version(),
					 'PROJECT_AUTHORS': ', '.join(authors),
					 'PROJECT_COPYRIGHT': 'Copyright (c) ' + year + ' ' + ', '.join(authors),
					 'PROJECT_LICENSE': ', '.join(meson.project_license())})
endif
