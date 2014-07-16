/*
 * plot.c
 *
 *  Created on: Jul 16, 2014
 *      Author: aarias
 */

#include "gnuplot_i.h"

/*typedef struct tot_event_per_actor {
	char *plot_file;
	char *event_name[];
} action_s;*/

int main(int argc, char **argv) {

	if (argc == 1){
		printf("PapiPlot usage:\n"
				"%s {path}\n"
				"being {path} the path to your papify-output generated folder\n"
				"\nIf not installed, you need to install gnuplot. In Ubuntu run:\n"
				"sudo apt-get install gnuplot\n",
				argv[0]);
		exit(0);
	}
	char* path = malloc(strlen(argv[1]));
	strcpy(path, argv[1]);

	plot(path);

	return 0;
}

void plot(char *path) {
	gnuplot_ctrl * h ;
	h = gnuplot_init() ;

	int n = 2; //number of events


	gnuplot_cmd(h, "reset");
	gnuplot_cmd(h, "dx=5.");
	gnuplot_cmd(h, "n=%d", n);
	gnuplot_cmd(h, "total_box_width_relative=0.75");
	gnuplot_cmd(h, "gap_width_relative=0.1");
	gnuplot_cmd(h, "d_width=(gap_width_relative+total_box_width_relative)*dx/2.");
	gnuplot_cmd(h, "reset");

	gnuplot_cmd(h,"list = ''");
	gnuplot_cmd(h,"index(w) = words(substr(list, 0, strstrt(list, w)-1))");
	gnuplot_cmd(h,"add_label(d) = (strstrt(list, d) == 0 ? list=list.' '.d : '')");
	gnuplot_cmd(h, "set datafile separator \";\"");
	gnuplot_cmd(h, "set xtics rotate");
	//gnuplot_cmd(h, "set logscale y");
	gnuplot_cmd(h, "plot \"%s\" "
			"every ::1 using (d='|'.strcol(2).'|', add_label(d),index(d)):3:xtic(2) smooth frequency w boxes lc rgb\"green\" title \"PAPI TOT INS\"",
			path);



/*	plot "profit.dat" u 1:2 w boxes lc rgb"green" notitle,\
	     "profit.dat" u ($1+d_width):3 w boxes lc rgb"red" notitle
	     http://gnuplot-surprising.blogspot.com.es/2011/09/plot-histograms-using-boxes.html
	     http://gnuplot-surprising.blogspot.com.es/2011/10/add-value-labels-to-top-of-bars-in-bar.html
	     */
	sleep(50000000);
}
