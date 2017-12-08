###############
# executables #
###############

# fastpool
if not meson.is_subproject()
  fastpool_mains = executable('fastpool', files(['pool.cpp']),
  install : true,
  dependencies : fastpool_lib_deps,
  include_directories : fastpool_include_directories,
  cpp_args : fastpool_flags)
endif