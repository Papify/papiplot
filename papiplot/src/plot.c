/*
 * plot.c
 *
 *  Created on: Jul 16, 2014
 *      Author: aarias
 */

#include "gnuplot_i.h"
#include <dirent.h>

typedef struct event_acum_ {
	int events_nb;
	unsigned long int *acum;
	char *event_name[];
} event_acum_;

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

	search_and_plot(path);
	//plot(path);

	return 0;
}

void plot(char *path, char* actor_name) {
	int i;

	gnuplot_ctrl * h ;
	h = gnuplot_init() ;

	gnuplot_cmd(h,"list = ''");
	gnuplot_cmd(h,"index(w) = words(substr(list, 0, strstrt(list, w)-1))");
	gnuplot_cmd(h,"add_label(d) = (strstrt(list, d) == 0 ? list=list.' '.d : '')");

	gnuplot_cmd(h, "set datafile separator \";\"");
	gnuplot_cmd(h, "set xtics rotate by -35");

	int number_of_actions = 4; //TODO function that returns number of actions
	int number_of_events = get_events_nb(path);

	gnuplot_cmd(h, "set term png size 1680,1050");
	gnuplot_cmd(h, "set output \"bar_labels.png\"");
	gnuplot_cmd(h, "set xlabel \"Actions\"");
	gnuplot_cmd(h, "set ylabel \"Number of occurrences\"");


	gnuplot_cmd(h, "dx=0.7");
	gnuplot_cmd(h, "n=%d",number_of_events);
	gnuplot_cmd(h, "total_box_width_relative=0.3");
	gnuplot_cmd(h, "gap_width_relative=0.1");
	gnuplot_cmd(h, "d_width=(gap_width_relative+total_box_width_relative)*dx/2.");
	gnuplot_cmd(h, "set boxwidth total_box_width_relative/n relative");
	gnuplot_cmd(h, "set style fill transparent solid 0.5 noborder");


	gnuplot_cmd(h, "set xrange [-1:%d]", number_of_actions);
	gnuplot_cmd(h, "set title \"Events in %s per action\"", actor_name);
	gnuplot_cmd(h, "set xtics out");
	gnuplot_cmd(h, "set xtics nomirror");
	gnuplot_cmd(h, "set xtics autofreq");
	gnuplot_cmd(h, "set xtics font \"Verdana,8\"");
	gnuplot_cmd(h, "set label rotate by 90");
	gnuplot_cmd(h, "set grid");

	/*gnuplot_cmd(h, "set xtics(\\");
	for (i=0;i<(number_of_actions-1);i++)
		gnuplot_cmd(h,"\"\" %d,\\",i);
	gnuplot_cmd(h,"\"\" %d)",i);*/


	gnuplot_cmd(h, "plot \\");
	if(number_of_events==1){
		i = 0;
		gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+(d_width*%d)):%d:xtic(2) smooth frequency w boxes title columnhead,\\", path, i, i+3);
		//gnuplot_cmd(h, "\"%s\" using (d='|'.strcol(2).'|', add_label(d),index(d)):3:3 smooth frequency w labels rotate by 90 font \",5\" notitle", path);
	}
	else if (number_of_events==0){
		printf("nothing to plot\n");
		exit(0);
	}
	else{
		for(i=0;i<number_of_events;i++){
			if(i == 0){//first
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+(d_width*%d)):%d:xtic(2) smooth frequency w boxes title columnhead,\\", path, i, i+3);
				//gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+(d_width*%d)):%d:%d smooth frequency w labels rotate by 90 font \",5\" notitle,\\", path,i,i+3,i+3);
			}
			else if (i==(number_of_events-1)){//last
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+(d_width*%d)):%d smooth frequency w boxes title columnhead,\\", path, i, i+3);
				//gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+d_width*%d):%d:%d smooth frequency w labels rotate by 90 font \",5\" notitle", path,i,i+3,i+3);
			}
			else
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d)),index(d)+(d_width*%d)):%d smooth frequency w boxes title columnhead,\\", path, i, i+3);
				//gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+(d_width*%d)):%d:%d smooth frequency w labels rotate by 90 font \",5\" notitle,\\", path,i,i+3,i+3);
		}
	}

	gnuplot_close(h);


//	gnuplot_cmd(h, "plot \\");

//	gnuplot_cmd(h, "\"%s\" using (d='|'.strcol(2).'|', add_label(d),index(d)):3:xtic(2) smooth frequency w boxes title columnhead,\\", path);
//	gnuplot_cmd(h, "\"%s\" using (d='|'.strcol(2).'|', add_label(d),index(d)):3:3 smooth frequency w labels rotate by 90 font \",5\" notitle,\\", path);

//	gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+d_width):4 smooth frequency w boxes title columnhead,\\", path);
//	gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+d_width):4:4 smooth frequency w labels rotate by 90 font \",5\" notitle,\\", path);

//	gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+d_width*2):5 smooth frequency w boxes title columnhead,\\", path);
//	gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(2).'|', add_label(d),index(d))+d_width*2):5:5 smooth frequency w labels rotate by 90 font \",5\" notitle", path);


//	gnuplot_close(h);
//	sleep(100);


/*	plot "profit.dat" u 1:2 w boxes lc rgb"green" notitle,\
	     "profit.dat" u ($1+d_width):3 w boxes lc rgb"red" notitle
	     http://gnuplot-surprising.blogspot.com.es/2011/09/plot-histograms-using-boxes.html
	     http://gnuplot-surprising.blogspot.com.es/2011/10/add-value-labels-to-top-of-bars-in-bar.html
	     */
	sleep(50000000);
}

int get_events_nb(char *path) {
	char buf[1500];
	char *token;
	int nb = 0;
	FILE* file = fopen(path,"r");

	fgets(buf,1500,file);

	token = strtok(buf,";");

	while(token !=NULL){
		nb++;
		token = strtok(NULL,";");
	}

	fclose(file);
	return nb-3;//-3 because of first two columns (not events) and bc line ends with ';'
}

const int is_papioutput(const char *filename) {
	if (strstr(filename,"papi_output") != NULL) return 1; //si el fichero no tiene extensión..
    else return 0;
}

void search_and_plot(char *path) {

    struct dirent *pDirent;
    DIR *pDir;
    int success = 0;
    pDir = opendir (path);
    if (pDir == NULL) {
        perror ("Cannot open directory\n");
        exit(1);
    }
    char * path_to_csv;

    while ((pDirent = readdir(pDir)) != NULL) {
    	//usleep(30000);
    	if(is_papioutput(pDirent->d_name)) {
    		path_to_csv = malloc(strlen(path)+strlen(pDirent->d_name)+2);
    		strcpy(path_to_csv, path);
    		strcat(path_to_csv,"/");
    		strcat(path_to_csv,pDirent->d_name);
        	printf("plot of %s\n", pDirent->d_name);
        	//plot(path_to_csv, pDirent->d_name);
        	process_data(path, path_to_csv, pDirent->d_name);
        	break;//temp..
        }
    }
    printf("out of the loooop!\n");
    return;
}

void process_data (char* path_todir, char* path_tofile, char* filename){
	char buf[1500];
	char *token;
	int nb = 0;
	char *saveptr;
	event_acum_* acum;
	FILE* ofile = fopen(path_tofile,"r");//original csv
	char* path_todatafile = malloc(strlen(path_todir)+strlen("/papidata.temp")+4);
	char* actor_name = &(*filename)+12;
	printf("actor name is %s\n", actor_name);
	strcpy(path_todatafile, path_todir);
	strcat(path_todatafile,"/");
	strcat(path_todatafile,"plotdata_");
	strcat(path_todatafile,actor_name);
	printf("creating %s\n", path_todatafile);
	FILE* datafile = fopen(path_todatafile,"w");

	int columns = get_events_nb(path_tofile)+2;
	acum = malloc(sizeof(event_acum_)+sizeof(char*)*(columns-2));
	acum->events_nb = get_events_nb(path_tofile);
	acum->acum = malloc(sizeof(long int)*get_events_nb(path_tofile));
	int i;
	for(i = 0; i <acum->events_nb; i++){
		acum->acum[i]=0;
	}

	fgets(buf,1500,ofile);//read first line
	token = strtok(buf,";");
	token = strtok(NULL,";");//skip action column
	for(i=0;i<acum->events_nb;i++){
		token = strtok(NULL,";");
		acum->event_name[i] = malloc(strlen(token)+1);
		strcpy(acum->event_name[i], token);
		printf("%s saved\n", acum->event_name[i]);
	}

	//do{
		fgets(buf,1500,ofile);
		token = strtok(buf,";");
		token = strtok(NULL,";");//skip actor name
		token = strtok(NULL,";");//skip action name
		for(i=0;i<acum->events_nb;i++){
			acum->acum[i] += atol(strtok_r(token,"\"", &saveptr));
			printf("evento %s, se suma: %s, total = %lu\n", acum->event_name[i], token, acum->acum[i]);
			token = strtok(saveptr,";");
		}



	//}while(buf != NULL);
}

