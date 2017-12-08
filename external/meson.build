###############
# ext headers #
###############

if not meson.is_subproject()
  install_headers(
    files([
      'blockingconcurrentqueue.h',
      'concurrentqueue.h']),
    subdir : 'concurrentqueue')
endif

fastpool_ext_include_directories = include_directories('.')
