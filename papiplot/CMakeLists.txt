project(papiplot)
cmake_minimum_required(VERSION 2.8)

set(papiplot_src
	src/gnuplot_i.h
	src/gnuplot_i.c
	src/plot.c
)
add_executable(papiplot ${papiplot_src})
set(test_src
	test/anim.c
	test/example.c
	test/sinepng.c
	${papiplot_src}
)
add_executable(papiplot-test ${test_src})

install(TARGETS papiplot
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
