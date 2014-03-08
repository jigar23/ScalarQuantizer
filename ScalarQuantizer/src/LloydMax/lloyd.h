/*
 * lloyd.h
 *
 *  Created on: Feb 21, 2014
 *      Author: jigar
 */

#ifndef LLOYD_H_
#define LLOYD_H_
#include <iostream>
#include <math.h>
#include "../FileIO/fileIO.h"
#include "../global.h"

using namespace std;


#define MIN_VAL 0
#define MAX_VAL 255

void train(char *memblock, int n);
unsigned char * test(char *memblock, int fileSize);
void initialize();
void printT();
void printR();
void setLevel(int n);
unsigned short* getPixelFrequency(char *memblock);
void writeHistogramdata(unsigned short* P);
void calculateIntervals();
void getNewLevels(unsigned short *P);
float getQuantizedValue(float key);
float getMSE(char *memblock);
float getDistortionRate(float previousMSE, float currentMSE);


#endif /* LLOYD_H_ */
