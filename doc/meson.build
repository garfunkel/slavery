doxygen = find_program('doxygen', required: false)

if doxygen.found()
	run_target('doc',
	           command: [doxygen, '@CURRENT_SOURCE_DIR@/Doxyfile'],
			   env: {'PROJECT_NAME': meson.project_name(), 'PROJECT_BRIEF': description})
endif
