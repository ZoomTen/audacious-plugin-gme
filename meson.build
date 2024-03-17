project('audacious-plugin-gme', 'c', 'cpp',
        version: '4.3.1',
        meson_version: '>= 0.51',
        default_options: [
          'c_std=gnu99',
          'cpp_std=gnu++17',
          'warning_level=1'
        ])


copyright = 'Copyright (C) 2001-2023 Audacious developers and others'


have_darwin = host_machine.system() == 'darwin'
have_windows = host_machine.system() == 'windows'


cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

if have_darwin
  add_languages('objc', 'objcpp')

  objc = meson.get_compiler('objc')
  objcpp = meson.get_compiler('objcpp')
endif


audacious_req = '>= 4.2'
audacious_dep = dependency('audacious', version: audacious_req, required: true)


# XXX - make this its own .pc file
audtag_dep = declare_dependency(link_args: [
  '-L@0@'.format(audacious_dep.get_variable(pkgconfig: 'lib_dir')),
  '-laudtag'
])


# XXX - make this its own .pc file
audqt_dep = declare_dependency(link_args: [
  '-L@0@'.format(audacious_dep.get_variable(pkgconfig: 'lib_dir')),
  '-laudqt'
])


# XXX - make this its own .pc file
audgui_dep = declare_dependency(link_args: [
  '-L@0@'.format(audacious_dep.get_variable(pkgconfig: 'lib_dir')),
  '-laudgui'
])


glib_req = '>= 2.32'
glib_dep = dependency('glib-2.0', version: glib_req, required: true)
gmodule_dep = dependency('gmodule-2.0', version: glib_req, required: true)
zlib_dep = dependency('zlib', required: true)

math_dep = cxx.find_library('m', required: false)
samplerate_dep = dependency('samplerate', required: false)
xml_dep = dependency('libxml-2.0', required: false)


if get_option('qt')
  if get_option('qt6')
    qt_req = '>= 6.0'
    qt_dep = dependency('qt6', version: qt_req, required: true, modules: ['Core', 'Widgets', 'Gui'])
  else
    qt_req = '>= 5.2'
    qt_dep = dependency('qt5', version: qt_req, required: true, modules: ['Core', 'Widgets', 'Gui'])
  endif
endif


if get_option('gtk')
  if get_option('gtk3')
    gtk_req = '>= 3.22'
    gtk_dep = dependency('gtk+-3.0', version: gtk_req, required: true)
  else
    gtk_req = '>= 2.24'
    gtk_dep = dependency('gtk+-2.0', version: gtk_req, required: true)
  endif
endif


if cc.get_id() in ['gcc', 'clang']
  common_flags = [
    '-ffast-math',
    '-Wtype-limits',
    '-Wno-stringop-truncation',
    '-fvisibility=hidden'
  ]

  cxx_flags = [
    '-Wno-non-virtual-dtor',
    '-Woverloaded-virtual'
  ]

  check_cflags = common_flags
  check_cxxflags = common_flags + cxx_flags

  add_project_arguments(cc.get_supported_arguments(check_cflags), language: 'c')
  add_project_arguments(cxx.get_supported_arguments(check_cxxflags), language: 'cpp')

  if have_darwin
    add_project_arguments(objc.get_supported_arguments(check_cflags), language: 'objc')
    add_project_arguments(objcpp.get_supported_arguments(check_cxxflags), language: 'objcpp')
  endif
endif


conf = configuration_data()
conf.set_quoted('COPYRIGHT', copyright)
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('VERSION', meson.project_version())
conf.set('PACKAGE_VERSION', meson.project_version())
if host_machine.endian() == 'big'
  conf.set10('WORDS_BIGENDIAN', true)
  conf.set10('BLARGG_BIG_ENDIAN', true)
else
  conf.set10('BLARGG_LITTLE_ENDIAN', true)
endif


# XXX - investigate to see if we can do better
if have_windows
  conf.set_quoted('PLUGIN_SUFFIX', '.dll')
elif have_darwin
  conf.set_quoted('PLUGIN_SUFFIX', '.dylib')
else
  conf.set_quoted('PLUGIN_SUFFIX', '.so')
endif


if have_windows
  conf.set('EXPORT', '__declspec(dllexport)')
elif cxx.has_argument('-fvisibility=default')
  conf.set('EXPORT', '__attribute__((visibility("default")))')
else
  error('Could not define EXPORT keyword for public symbols.')
endif


if meson.version().version_compare('>= 0.59')
  intl_dep = dependency('intl', required: false)
  intl_found = intl_dep.found()
else
  if cxx.has_function('ngettext', prefix: '#include <libintl.h>')
    intl_dep = dependency('', required: false)
    intl_found = true
  else
    intl_dep = cxx.find_library('intl', required: false)
    intl_found = intl_dep.found()
  endif
endif

if intl_found
  conf.set10('HAVE_GETTEXT', true)
endif


install_plugindir = audacious_dep.get_variable(pkgconfig: 'plugin_dir')


conf.set_quoted('INSTALL_PLUGINDIR', install_plugindir)
conf.set('plugindir', install_plugindir)


input_plugin_dir = join_paths(install_plugindir, 'Input')


if get_option('qt')
  conf.set10('USE_QT', true)
endif


if get_option('gtk')
  conf.set10('USE_GTK', true)
  if get_option('gtk3')
    conf.set10('USE_GTK3', true)
  endif
endif


if get_option('qt') or get_option('gtk')
  conf.set10('USE_GTK_OR_QT', true)
endif


subdir('src')
# subdir('po')


if meson.version().version_compare('>= 0.53')
  summary({
    'Prefix': get_option('prefix'),
    'Bin dir': get_option('bindir'),
    'Lib dir': get_option('libdir'),
    'Data dir': get_option('datadir'),
  }, section: 'Directories')

  summary({
    'Qt 5 support': get_option('qt') and not get_option('qt6'),
    'Qt 6 support': get_option('qt6'),
    'GTK2 support': get_option('gtk') and not get_option('gtk3'),
    'GTK3 support': get_option('gtk3'),
  }, section: 'GUI Toolkits')
endif
