src_inc = include_directories('.')


config_h_full_path = join_paths(meson.current_build_dir(), 'config.h')


if cc.get_id() in ['gcc', 'clang']
  add_project_arguments('-include', config_h_full_path, language: ['c', 'cpp'])

  if have_darwin
    add_project_arguments('-include', config_h_full_path, language: ['objc', 'objcpp'])
  endif
else
  error('Please implement -include handling for your chosen compiler.')
endif


# input plugins
subdir('plugin')

# config.h stuff
configure_file(input: 'config.h.meson',
  output: 'config.h',
  configuration: conf)
