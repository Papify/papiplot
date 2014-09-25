project(papiplot)

add_executable(papiplot
	src/gnuplot_i.h
	src/gnuplot_i.c
	src/plot.c
)

install(TARGETS papiplot
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
