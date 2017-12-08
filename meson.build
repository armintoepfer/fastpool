project(
  'fastpool',
  'cpp',
  version : '0.0.1',
  default_options : [
    'buildtype=release',
    'default_library=static',
    'warning_level=3',
    'cpp_std=c++14'],
  license : 'BSD-3',
  meson_version : '>= 0.41.1')

############
# CXXFLAGS #
############

fastpool_flags = []
cpp = meson.get_compiler('cpp')
foreach cflag: [
  '-Wno-terminate',
  '-Wno-missing-field-initializers']
    if cpp.has_argument(cflag)
      fastpool_flags += cflag
    endif
endforeach

################
# dependencies #
################

# threads
fastpool_thread_dep = dependency('threads', required : true)


fastpool_lib_deps = [
  fastpool_thread_dep]

###########
# headers #
###########

subdir('include')
subdir('external')

fastpool_include_directories = [
  fastpool_int_include_directories,
  fastpool_ext_include_directories]

###########
# example #
###########

subdir('example')

###################
# dependency info #
###################

if not meson.is_subproject()
  import('pkgconfig').generate(
    version : meson.project_version(),
    name : 'fastpool',
    filebase : 'fastpool',
    description : 'fastpool')
endif

fastpool_dep = declare_dependency(
  include_directories : fastpool_include_directories,
  dependencies : fastpool_lib_deps,
  version : meson.project_version(),
  compile_args : fastpool_flags)
