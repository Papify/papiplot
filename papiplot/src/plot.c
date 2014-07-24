/*
 * plot.c
 *
 *  Created on: Jul 16, 2014
 *      Author: aarias
 */

#include "gnuplot_i.h"
#include <dirent.h>
typedef struct event_acumulator {
	char* event_name;
	unsigned long long count;
}event_acumulator;

typedef struct event_acum_for_action {
	char* action_name;
	int events_nb;
	event_acumulator *acumulator[];
}event_acum_for_action;

typedef struct event_acum_for_actor {
	char *proc_file_path;
	char *actor_name;
	int actions_nb;
	event_acum_for_action *actions[];
} event_acum_for_actor;

event_acum_for_actor* process_data (char* path_todir, char* path_tofile, char* filename, char* path_to_totals);
int get_nb_of_actors_in_dir(char* path);
unsigned long long get_number(char *line);
void search_and_plot(char *path);
void struct_test(struct event_acum_for_actor *data);
int get_events_nb(char *path);
int get_actions_nb_in_file(char* path_tofile);
int NOLABELS;
unsigned int res_x;
unsigned int res_y;
int tics_font_size;
int labels_font_size;

void printhelp(char** argv){
	printf("\tPapiPlot options:\n"
			"\t-p [path]"
			"\t Set path to the papi-output folder generated with Papify. If not specified, current directory will be used.\n\n"
			"\t-x [res_x]"
			"\t Change horizontal resolution. If not specified, default is 1680.\n\n"
			"\t-y [res_y]"
			"\t Change vertical resolution. If not specified, default is 1050.\n\n"
			"\t-t [font size]"
			"\t Change tics font size. If not specified, default is 8.\n\n"
			"\t-l [font size]"
			"\t Change labels font size. If not specified, default is 8.\n\n"
			"\t-n \t \tNo labels will be printed\n\n"
			"\t-h \t \tPrint help\n\n"
			"\tPapiPlot requires GNUPLOT. If not installed, assuming you are on Ubuntu, run:\n"
			"\tsudo apt-get install gnuplot\n\n");
}


int main(int argc, char **argv) {
	int c, len;
	res_x = 1680;
	res_y = 1050;
	tics_font_size = 8;
	labels_font_size = 8;
	char* path = NULL;
	char* temp;

	while ((c = getopt (argc, argv, "l:t:nhp:x:y:")) != -1) {
			switch (c)
			{
				case 'h':
					printhelp(argv);
					exit(0);
				case 'x':
					res_x = atoi(optarg);
					break;
				case 'y':
					res_y = atoi(optarg);
					break;
				case 'l':
					labels_font_size = atoi(optarg);
					break;
				case 't':
					tics_font_size = atoi(optarg);
					break;
				case 'p':
					temp = optarg;
					len = strlen(temp)+2;
					path = malloc(len);
					strcpy(path, temp);
					path[len-2]='/';
					path[len-1]='\0';
					break;
				case 'n':
					NOLABELS = 1;
					break;
				case '?':
					if (isprint (optopt)) {
						fprintf (stderr, "Unknown option '-%c'.\n", optopt);
					}
					else {
						fprintf (stderr,
								"Unknown option character `\\x%x'.\n",
								optopt);
					}
					return 1;
				default:
					printhelp(argv);
					abort();
			}
		}

	if(path==NULL){
		char cwd[3000];
		getcwd(cwd, sizeof(cwd));
		len = strlen(cwd)+2;
		path = malloc(len);
		strcpy(path, cwd);
		path[len-2]='/';
		path[len-1]='\0';
	}

	search_and_plot(path);

	return 0;
}

void configure_handle(gnuplot_ctrl *  h, int number_of_events, int number_of_elements){

	gnuplot_cmd(h,"reset");

	gnuplot_cmd(h, "dx=.7");
	gnuplot_cmd(h, "n=%d", number_of_events);
	gnuplot_cmd(h, "total_box_width_relative=0.3");
	gnuplot_cmd(h, "gap_width_relative=0.1");
	gnuplot_cmd(h, "d_width=(gap_width_relative+total_box_width_relative)*dx/2.");

	gnuplot_cmd(h,"list = ''");
	gnuplot_cmd(h,"index(w) = words(substr(list, 0, strstrt(list, w)-1))");
	gnuplot_cmd(h,"add_label(d) = (strstrt(list, d) == 0 ? list=list.' '.d : '')");

	gnuplot_cmd(h, "set datafile separator \";\"");
	gnuplot_cmd(h, "set xtics rotate by -25");
	gnuplot_cmd(h, "set logscale y");

	gnuplot_cmd(h, "set term png truecolor");
	gnuplot_cmd(h, "set grid");
	gnuplot_cmd(h, "set boxwidth total_box_width_relative/n relative");
	gnuplot_cmd(h, "set style fill transparent solid 0.5 noborder");


	gnuplot_cmd(h, "set term png size %d,%d", res_x, res_y);

	gnuplot_cmd(h, "set ylabel \"Number of occurrences\"");


	/*
	gnuplot_cmd(h, "set xrange [-1:%d]", number_of_elements);
*/
	gnuplot_cmd(h, "set xtics out");
	gnuplot_cmd(h, "set xtics nomirror");
	gnuplot_cmd(h, "set xtics autofreq");
	gnuplot_cmd(h, "set xtics font \"monospace,%d\"", tics_font_size);
	gnuplot_cmd(h, "set tic scale 0");
	gnuplot_cmd(h, "set grid");

	gnuplot_cmd(h, "set label at character 1,1 \"Generated with Papify\"");
	gnuplot_cmd(h, "set format y \"%%.0se%%S\"");

}

void plot_overall(char* output_path, char *path, int number_of_actors, int number_of_events) {
	int i;
	gnuplot_ctrl * h ;
	h = gnuplot_init() ;

	//int label_rot = 20;
	configure_handle(h, number_of_events, number_of_actors);
	gnuplot_cmd(h, "set output \"%s/papiplot_overall_.png\"", output_path);
	gnuplot_cmd(h, "set title \"Events per actor\"");
	gnuplot_cmd(h, "set xlabel \"Actors\"");


	//draw lines:
	gnuplot_cmd(h, "plot \\");
	if(number_of_events==1){
		i = 0;
		gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:xtic(1) w boxes title columnhead,\\", path, i, i+2);
	}
	else if (number_of_events==0){
		printf("nothing to plot\n");
		exit(0);
	}
	else{
		for(i=0;i<number_of_events;i++){
			if(i == 0){//first
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:xtic(1) w boxes title columnhead,\\", path, i, i+2);
				}
			else if (i==(number_of_events-1)){//last
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d w boxes title columnhead,\\", path, i, i+2);
			}
			else{
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d w boxes title columnhead,\\", path, i, i+2);
			}
		}
	}
	//draw numbers:
	//gnuplot_cmd(h, "plot \\");
	if (NOLABELS)
		gnuplot_cmd(h, "0 notitle"); //end plot here
	else
	{
		if(number_of_events==1){
			i = 0;
			gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \",5\" notitle", path, i, i+2, i+2);
		}
		else if (number_of_events==0){
			printf("nothing to plot\n");
			exit(0);
		}
		else{
			for(i=0;i<number_of_events;i++){
				if(i == 0){//first
					gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \"monospace,%d\" notitle,\\", path, i, i+2, i+2, labels_font_size);
					}
				else if (i==(number_of_events-1)){//last
					gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \"monospace,%d\" notitle", path, i, i+2, i+2, labels_font_size);
				}
				else{
					gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \"monospace,%d\" notitle,\\", path, i, i+2, i+2, labels_font_size);
				}
			}
		}
	}
	gnuplot_close(h);
}

void plot(char* output_path, char *path, char* actor_name, int number_of_actions, int number_of_events) {
	int i;
	gnuplot_ctrl * h ;
	h = gnuplot_init() ;

	//int label_rot = 20;
	configure_handle(h, number_of_events, number_of_actions);
	gnuplot_cmd(h, "set output \"/%s/papiplot_%s.png\"", output_path, actor_name);
	gnuplot_cmd(h, "set title \"Events in actor \\\"%s\\\" per action\"", actor_name);
	gnuplot_cmd(h, "set xlabel \"Actions\"");



	//draw lines:
	gnuplot_cmd(h, "plot \\");
	if(number_of_events==1){
		i = 0;
		gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:xtic(1) w boxes title columnhead,\\", path, i, i+2);
	}
	else if (number_of_events==0){
		printf("nothing to plot\n");
		exit(0);
	}
	else{
		for(i=0;i<number_of_events;i++){
			if(i == 0){//first
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:xtic(1) w boxes title columnhead,\\", path, i, i+2);
				}
			else if (i==(number_of_events-1)){//last
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d w boxes title columnhead,\\", path, i, i+2);
				//gnuplot_cmd(h, "0 notitle"); //end plot here
			}
			else{
				gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d w boxes title columnhead,\\", path, i, i+2);
			}
		}
	}
	//draw numbers:
	//gnuplot_cmd(h, "plot \\");
	if (NOLABELS)
		gnuplot_cmd(h, "0 notitle"); //end plot here
	else
	{
		if(number_of_events==1){
			i = 0;
			gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \",5\" notitle", path, i, i+2, i+2);
		}
		else if (number_of_events==0){
			printf("nothing to plot\n");
			exit(0);
		}
		else{
			for(i=0;i<number_of_events;i++){
				if(i == 0){//first
					gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \"monospace,%d\" notitle,\\", path, i, i+2, i+2, labels_font_size);
					}
				else if (i==(number_of_events-1)){//last
					gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \"monospace,%d\" notitle", path, i, i+2, i+2, labels_font_size);
				}
				else{
					gnuplot_cmd(h, "\"%s\" using ((d='|'.strcol(1).'|', add_label(d),index(d))+(d_width*%d)):%d:(gprintf(\"%%.2se%%S\", $%d)) w labels rotate by 20 font \"monospace,%d\" notitle,\\", path, i, i+2, i+2, labels_font_size);
				}
			}
		}
	}

	gnuplot_close(h);
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
		if(strcmp(token,"\n")==0) nb--;
		token = strtok(NULL,";");
	}

	fclose(file);
	return nb-2;//-2 because of the first two columns (not events)
}

const int is_papioutput(const char *filename) {
	if (strstr(filename,"papi_output") != NULL) return 1; //si el fichero no tiene extensión..
    else return 0;
}

void html_init(FILE* htmlfile){
	fprintf(htmlfile, "<html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\"><title>PAPI stats</title></head>"
			"<body><h1>Papify stats processed with PapiPlot</h1><hr style=\"width: 100%; height: 2px;\">");
}
void html_close(FILE* htmlfile, char* path){
    fprintf(htmlfile, "<h2>Overall: Events per actor</h2><img alt=\"\""
    		"src=\"papiplot_overall_.png\">");



    int events_nb = get_events_nb(path)+1;
    int i, j;
    char* token;
    char buf[1500];

    FILE* ofile = fopen(path, "r");

	fprintf(htmlfile,"<table border=\"1\">"
	"<td>Actor<br></td>");
	fgets(buf, 1500, ofile);
	token = strtok(buf,";");
	for(i=0;i<events_nb;i++){
		token = strtok(NULL,";");
		fprintf(htmlfile,"<td>%s<br></td>\n", token);
	}
	fprintf(htmlfile,"</tr>\n");

	int actors_nb = get_actions_nb_in_file(path);
	for(i=0;i<actors_nb;i++){
		fprintf(htmlfile,"<tr>\n");
		fgets(buf, 1500, ofile);
		token = strtok(buf,";");
		for(j=0;j<events_nb+1;j++){
				fprintf(htmlfile,"<td>%s<br></td>\n", token);
				token = strtok(NULL,";");
		}
		fprintf(htmlfile,"</tr>\n");
	}

	fprintf(htmlfile,"</table>\n");
	fprintf(htmlfile,"<hr style=\"width: 100%; height: 2px;\">\n");
	fprintf(htmlfile,"<br>\n");


	fprintf(htmlfile, "</body></html>");
	fclose(ofile);
}

void html_actor(FILE* htmlfile, event_acum_for_actor* data){
	fprintf(htmlfile,"<table style=\"text-align: left; width: 100%;\" border=\"1\" cellpadding=\"2\""
			"cellspacing=\"2\">\n");

	fprintf(htmlfile,"<table border=\"1\">"
			"<td>Action<br></td>");


	int i, j;
	fprintf(htmlfile, "<h2>Stats for actor \"%s\"</h2>\n",data->actor_name);

	fprintf(htmlfile, "<img alt=\"\""
		    		"src=\"papiplot_%s.png\">\n",data->actor_name);


	for(i=0; i<data->actions[0]->events_nb;i++){
		fprintf(htmlfile,"<td>%s<br></td>\n", data->actions[0]->acumulator[i]->event_name);
	}
	fprintf(htmlfile,"</tr>\n");


	for(i=0; i<data->actions_nb;i++){
		fprintf(htmlfile,"<tr>\n");
		fprintf(htmlfile,"<td>%s<br></td>\n", data->actions[i]->action_name);
		for(j=0; j<data->actions[i]->events_nb;j++){
			fprintf(htmlfile,"<td>%llu<br></td>\n", data->actions[i]->acumulator[j]->count);
		}
		fprintf(htmlfile,"</tr>\n");
	}
	fprintf(htmlfile,"</table>\n");
	fprintf(htmlfile,"<hr style=\"width: 100%; height: 2px;\">\n");
	fprintf(htmlfile,"<br>\n");
}

void search_and_plot(char *path) {

	char * path_to_totals = malloc(strlen(path)+strlen("plotdata_papi_totals.csv")+2);
	strcpy(path_to_totals, path);
	strcat(path_to_totals,"plotdata_papi_totals.csv");

	FILE* datafile = fopen(path_to_totals,"w");//reset
	fclose(datafile);

	char * html = malloc(strlen(path)+strlen("papiplot.html")+2);
	strcpy(html, path);
	strcat(html,"papiplot.html");
	FILE* htmlfile = fopen(html,"w");
	html_init(htmlfile);
	int actors_nb = get_nb_of_actors_in_dir(path);
	event_acum_for_actor* data;

    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir (path);
    if (pDir == NULL) {
        perror ("Cannot open directory\n");
        exit(1);
    }
    char * path_to_csv;

    while ((pDirent = readdir(pDir)) != NULL) {
    	if(is_papioutput(pDirent->d_name)) {
    		//printf("going for %s\n", pDirent->d_name);
    		path_to_csv = malloc(strlen(path)+strlen(pDirent->d_name)+2);
    		strcpy(path_to_csv, path);
    		strcat(path_to_csv,"/");
    		strcat(path_to_csv,pDirent->d_name);
        	data = process_data(path, path_to_csv, pDirent->d_name, path_to_totals);
        	plot(path, data->proc_file_path, data->actor_name, data->actions_nb, data->actions[0]->events_nb);
        	html_actor(htmlfile, data);
        	remove(data->proc_file_path);
        	free(data);
        }
    }
    //printf("going for overalls\n");
    plot_overall(path, path_to_totals, actors_nb, get_events_nb(path_to_totals)+1);

    html_close(htmlfile, path_to_totals);
    remove(path_to_totals);

    fclose(htmlfile);
    closedir (pDir);
    return;
}

event_acum_for_actor* process_data (char* path_todir, char* path_tofile, char* filename, char* path_to_totals){
	char buf[1500];
	char *token;
	event_acum_for_actor* actor;
	char* path_todatafile = malloc(strlen(path_todir)+strlen("/papidata.temp")+4);
	char* actor_name = &(*filename)+12;
	//printf("actor name is %s\n", actor_name);
	strcpy(path_todatafile, path_todir);
	strcat(path_todatafile,"/");
	strcat(path_todatafile,"plotdata_");
	strcat(path_todatafile,actor_name);
	//printf("creating %s\n", path_todatafile);


	int events_nb = get_events_nb(path_tofile);
	//printf("we have %d actions here\n",get_actions_nb_in_file(path_tofile));/////////////////////////////////////////////
	//get_actions_nb_in_file(path_tofile);
	int actions_nb=get_actions_nb_in_file(path_tofile);

	actor = malloc(sizeof(event_acum_for_actor)*2+sizeof(event_acum_for_action*)*(actions_nb)*3);//weird memory is weird..
	actor->actor_name = malloc(strlen(actor_name));
	strcpy(actor->actor_name,actor_name);
	actor->proc_file_path = path_todatafile;
	actor->actions_nb = actions_nb;
	int i, j;


	for(i=0;i<actions_nb*3;i++){
		actor->actions[i] = malloc(sizeof(event_acum_for_action)*2+sizeof(event_acumulator)*events_nb*3);//weird memory is weird..
		actor->actions[i]->events_nb = events_nb;
		//actor->actions[i]->action_name
		for(j=0;j<events_nb;j++){
			actor->actions[i]->acumulator[j] = malloc(sizeof(event_acumulator));
			//actor->actions[i]->acumulator->event_name
		}
	}


	for(j = 0; j <actor->actions_nb; j++)
		for(i = 0; i <actor->actions[j]->events_nb; i++){
			actor->actions[j]->acumulator[i]->count = 0;
		}

	FILE* ofile = fopen(path_tofile, "r");
	fgets(buf,1500,ofile);//read first line
	token = strtok(buf,";");
	token = strtok(NULL,";");//skip action column (actot name)

	for(i=0;i<actor->actions[0]->events_nb;i++){
		token = strtok(NULL,";");
		for(j = 0; j <actor->actions_nb; j++){
			if(token[strlen(token)-1]=='\n') token[strlen(token)-1] = '\0';
			actor->actions[j]->acumulator[i]->event_name = malloc(strlen(token)+1);
			strcpy(actor->actions[j]->acumulator[i]->event_name, token);
			//acum->acumulator->event_name[i] = malloc(strlen(token)+1);
			//strcpy(acum->acumulator->event_name[i], token);
		}
	}


	int found = -1;
	int found_actions = 0;
	int current;


	long unsigned int totals[events_nb];
	for (i=0;i<events_nb;i++){
		totals[i]=0;
	}


	//acumuating structures
	int result; //borrar
	while(fgets(buf,1500,ofile)!=NULL){
		token = strtok(buf,";");
		token = strtok(NULL,";");//read action name

		for(i=0; i<actor->actions_nb; i++){
			if(actor->actions[i]->action_name==NULL)
				break;
			else {
				result = strcmp(actor->actions[i]->action_name,token);
				if(result==0){
					found = i;
					break;
				}
				else {
				}
			}
		}
		if (found == -1){
			//printf("not found, adding action %s\n", token);
			actor->actions[found_actions]->action_name = malloc(strlen(token)+1);
			strcpy(actor->actions[found_actions]->action_name,token);
		}

		current = (found == -1)? found_actions++ : found;


		for(i = 0; i <actor->actions[current]->events_nb; i++){
			token = strtok(NULL,";");
			actor->actions[current]->acumulator[i]->count +=  get_number(token);
			totals[i] += get_number(token);
			//printf("adding %s (%llu).. totals[%d] = %llu\n", token, get_number(token), i, totals[i]);
		}

		found = -1;
	}


	fclose(ofile);

	//writing events per action
	FILE* datafile = fopen(path_todatafile,"w");
	fprintf(datafile,"Action;");
	for(i=0;i< events_nb;i++)
		fprintf(datafile,"%s;",actor->actions[0]->acumulator[i]->event_name);
	fprintf(datafile,"\n");

	for(i = 0; i< actions_nb;i++){
		fprintf(datafile,"%s;", actor->actions[i]->action_name);
		for(j=0; j < events_nb; j++){
			fprintf(datafile,"%llu;", actor->actions[i]->acumulator[j]->count);
		}
		fprintf(datafile,"\n");
	}
	fclose(datafile);

	//adding to totals per actor
	int label_set = 0;
	datafile = fopen(path_to_totals,"r");
	fgets(buf,1500,datafile);
	if(strstr(buf,"Actor;")!=NULL) label_set = 1;
	fclose(datafile);

	//appending to totals
	datafile = fopen(path_to_totals,"a");
	if(label_set == 0){
		fprintf(datafile,"Actor;");
		for(i=0;i< events_nb;i++)
			fprintf(datafile,"%s;",actor->actions[0]->acumulator[i]->event_name);
		fprintf(datafile,"\n");
	}

	fprintf(datafile,"%s;",strtok(actor->actor_name,"."));
	for(j=0; j < events_nb; j++){
		fprintf(datafile,"%lu;", totals[j]);
	}
	fprintf(datafile,"\n");
	fclose(datafile);


	return actor;
}

unsigned long long get_number(char *line){ //removes quotes from string number
	int lineLength = strlen(line);
	int j = 0;
	int i;
	for (i = 0; i < lineLength; i++)
	{
	    if (line[i] == '\\')
	    {
	        line[j++] = line[i++];
	        line[j++] = line[i];
	        if (line[i] == '\0')
	            break;
	    }
	    else if (line[i] != '"')
	        line[j++] = line[i];
	}
	line[j] = '\0';

	//char *tempptr=NULL;
	return strtoul(line,NULL,10);
}

//test structure
void struct_test(struct event_acum_for_actor *data){
	int i,j;
	printf("Testing structure\n");

	printf("Actor name: %s\n", data->actor_name);
	printf("Path to processed data file: %s\n", data->proc_file_path);
	printf("Actions number: %d\n", data->actions_nb);

	for(i = 0; i<data->actions_nb;i++){
		printf("\tAction name: %s\n", data->actions[i]->action_name);
		printf("\tNumber of events: %d\n", data->actions[i]->events_nb);
		for(j=0; j < data->actions[i]->events_nb; j++){
			printf("\t\tEvent: %s, total: %llu\n", data->actions[i]->acumulator[j]->event_name, data->actions[i]->acumulator[j]->count);
		}

			//printf("Event: %s, total: %lu\n",data->, data->acumulator->acum[i]);
	}
}

int get_nb_of_actors_in_dir(char* path){
	int actors_nb=0;
	struct dirent *pDirent;
	DIR *pDir;

	pDir = opendir (path);
	if (pDir == NULL) {
		perror ("Cannot open directory\n");
		exit(1);
	}

	while ((pDirent = readdir(pDir)) != NULL) {
		if(is_papioutput(pDirent->d_name)) {
			actors_nb++;
		}
	}

	closedir (pDir);

	return actors_nb;
}

int get_actions_nb_in_file(char* path_tofile){//weird memory allocation... ¿??¿
	int actions_nb = 0;
	char** theActions;
	char* token;
	char buf[1500];
	int i;
	int found = -1;
	theActions = malloc(0);

	FILE* ofile = fopen(path_tofile, "r");

	fgets(buf,1500,ofile);//skip labels lines
	while(fgets(buf,1500,ofile)!=NULL){
		token = strtok(buf,";");
		token = strtok(NULL,";");//read action name

		for(i=0; i<actions_nb; i++)
			if(theActions[i]!=NULL && strcmp(theActions[i],token)==0){
				found = i;
				break;
			}

		if (found == -1){
			theActions = realloc(theActions, sizeof(char*)*(actions_nb+2));
			theActions[actions_nb] = malloc(strlen(token)+1);
			theActions[actions_nb+1] = malloc(strlen(token)+1);
			strcpy(theActions[actions_nb],token);
			actions_nb++;
		}
		found = -1;
	}

	fclose(ofile);

	for(i=0; i<actions_nb; i++)
		free(theActions[i]);
	free(theActions);

	return actions_nb;
}
