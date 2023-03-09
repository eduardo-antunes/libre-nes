project('libre-nes', 'cpp',
  default_options: ['cpp_std=c++17']
)

inc_dir = include_directories('include')
sources = files(
  'src/main.cpp'      ,
  'src/emulator.cpp'  ,
  'src/processor.cpp' ,
)

executable('libre-nes', sources, 
  include_directories: inc_dir,
)
