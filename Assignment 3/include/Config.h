#ifndef CONFIG_H
#define CONFIG_H

#define SEARCH_WIDTH 32
#define SEARCH_HEIGHT 32

#define	DO_DCT true
#define	DO_QT	true
#define DO_MC true
#define DO_INTRA true

#define MV_TO_CSV true // Writes the motion vectors to a CSV file that can be visualised.
#define CSV_NAME "xxx\\data\\flower_50\\vectors.csv"
#define RES_NAME "xxx\\data\\results.txt"

#define CALC_DIFF(x, y) ( x - y ) * ( x - y )

#endif