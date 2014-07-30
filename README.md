PapiPlot
========

PapiPlot takes the output generated with ![Papify](https://github.com/alejoar/papify) and generates cute graphs you can use to evaluate the performance of your ORC-Apps. These graphs come served with a generated html where value tables are included for each graph, so you can check the specific value for each point. An example of an html generated with PapiPlot has been set up ![here](http://alejoar.github.io/papiplot/index.html).

PapiPlot generates the graphs ussing **gnuplot**, so make sure you have it installed:
```
sudo apt-get install gnuplot
```

If you place the PapiPlot binary inside the **papi-output** dir, you can run it without any arguments: it will generate the graphs with default parameters. Other options available are:
``` 
-p [path]	 Set path to the papi-output folder generated with Papify. If not specified, current directory will be used.

-x [res_x]	 Change horizontal resolution. If not specified, default is 1680.

-y [res_y]	 Change vertical resolution. If not specified, default is 1050.

-t [font size]	 Change tics font size. If not specified, default is 8.

-l [font size]	 Change labels font size. If not specified, default is 8.

-n 	 	No labels will be printed

-h 	 	Print help
```

========
Example:

./papiplot -n
![Alt text](papiplot/readme/papiplot_overall_.png?raw=true "Optional Title")


###Fancy some madness?

![MADNESS!](papiplot/readme/madness.png?raw=true "MADNESS!")
![MADNESS!](papiplot/readme/madness_not_labeled.png?raw=true "MADNESS!")


TODO:
More stats:
- Number of execs per action
- Number of execs per actor
- Average of event per action
- Average of event per actor


