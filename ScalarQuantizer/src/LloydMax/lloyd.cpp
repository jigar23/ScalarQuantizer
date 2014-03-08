/*
 * lloyd.cpp
 *
 *  Created on: Feb 21, 2014
 *      Author: jigar
 */

#include "lloyd.h"

namespace SQ {
	float *T;
	float *R;
	int L;
}

/**
 * This function sets the number of levels given
 * the number of bits.
 */
void setLevel(int n) {
	//L = 2^n
	SQ::L = pow(2,n);
}

/**
 * This function does the Lloyd Max training given
 * the read data and the number of bits.
 * It sets the quantization levels (T) and the centroids (R) in
 * the global file.
 */
void train(char *memblock, int n) {

	cout << "Started Training......" << endl;
	float previousMSE, currentMSE,PSNR, DistortionRate = 1;
	unsigned short *P;

	//Get the probabilities (COUNT)
	//	getProbablities();
	//Counts stored in P, P is of 16 bits, MAX count = 65536
	P = getPixelFrequency(memblock);

	//Write the Histogram data to a file.//Optional
	writeHistogramdata(P);

	//	SQ::L = pow(2,n);
	setLevel(n);

	//Initialize the centroids R.
	initialize();

	//First Iteration
	int iterationNo = 1;
	cout << "------------------- Iteration No. " << iterationNo++
					<< ". ----------------------" << endl;
	calculateIntervals();
	getNewLevels(P);
	printT();
	printR();
	currentMSE = getMSE(memblock);
	cout << "MSE for first Iteration is : " << currentMSE << endl;;

	PSNR = 10*log10(pow(255,2)/currentMSE);
	cout << "PSNR is: " << PSNR << endl;
	cout << "xxxxxxxxxxxxxxxxxxxxxx--End of Iteration " << iterationNo - 1
			<< ". xxxxxxxxxxxxxxxxxxxxxx" << endl << endl;

	//Recurse until distortion rate less than 0.001
	while (DistortionRate > 0.001) {
		cout << "------------------- Iteration No. " << iterationNo++
				<< ". ----------------------" << endl;
		calculateIntervals();
		getNewLevels(P);
		printT();
		printR();
		previousMSE = currentMSE;
		currentMSE = getMSE(memblock);
		cout << "Current MSE is: " << currentMSE << endl;
		DistortionRate = getDistortionRate(previousMSE, currentMSE);
		cout << "Distortion rate is : " << DistortionRate << endl;
		PSNR = 10*log10(pow(255,2)/currentMSE);
			cout << "PSNR is: " << PSNR << endl;
		cout << "xxxxxxxxxxxxxxxxxxxxxx--End of Iteration " << iterationNo - 1
				<< ". xxxxxxxxxxxxxxxxxxxxxx" << endl << endl;
	}

	cout << "Done training .. :)" << endl;
}

/*
 * This function has to run after the *train* function.
 * Training the file will create the quantization levels.
 * These quantization levels are used to find the nearest level in the
 * test image and achieve compression.
 * @param:
 * Input: 1. char *memblock - read data,
 * 2. int filesize - size of the data
 * Output: compressed data.
 */
unsigned char * test(char *memblock, int fileSize){

	cout <<endl << "----------- Quantizing the file ..." << endl;
	unsigned char *output = new unsigned char[fileSize];
	for(int i = 0; i < fileSize; i++){
		//Round the number to nearest integer
		unsigned char origvalue = memblock[i];
		unsigned char val = floor(getQuantizedValue(origvalue) + 0.5);
//		cout << i << " " << (int)val << endl;
		output[i] = val;
	}

	float MSE = getMSE(memblock);
	cout << "Mean Squared error is: " << MSE << endl;

	float PSNR = 10*log10(pow(255,2)/MSE);
	cout << "PSNR is: " << PSNR << endl;

	cout << "Done Quantizing. :) " << endl;
	cout << "Quantized file stored in *quantizedOutput.dat* file" << endl;
	return output;
}

/**
 * This function uniformly divides the total
 * range into the number of levels.
 *
 */
void initialize() {
	//Declaring the arrays
	//Observe their size
	SQ::R = new float[SQ::L];
	SQ::T = new float[SQ::L + 1];

	//Start and end intervals
	SQ::T[0] = 0;
	SQ::T[SQ::L] = 255;

	//uniform intervals
	float interval_size = (float) (MAX_VAL - MIN_VAL) / SQ::L;
//	cout << interval_size;
	for (int i = 0; i < SQ::L; i++) {
		//Placing SQ::R at the center of intervals
		SQ::R[i] = (i + 0.5) * interval_size;
//			cout << SQ::L;
	}

}

/**
 * This function calculates the frequency (prob) of each pixel.
 */
unsigned short* getPixelFrequency(char *memblock) {

	unsigned short *P = new unsigned short[256];
	//Initialize the count values with 0.

	for (int i = 0; i <= MAX_VAL; i++) {
		P[i] = 0;
	}
	unsigned char pixel;
	for (int i = 0; i < FileSizeinBytes; i++) {
		pixel = (unsigned char) memblock[i];
		P[pixel]++;
	}
//	//Just to check
//	long int sum = 0;
//	for(int i = 0; i < 256; i++){
//		sum += P[i];
//	}
//	cout << "SUM is : " << sum;
	return P;
}

/**
 * This function reads the frequency of each symbol
 * and writes the count of each symbol in the *histogram.dat* file.
 * The count is 16 bit. So while reading the file, read 16 bits
 * at a time to get count of pixel value 0 - 255.
 */
void writeHistogramdata(unsigned short* P) {

	cout << "Writing the histogram ..." << endl;
	//Data is written as follows:
	//Staring from 0 - 255
	//only counts of pixel are stored staring from 0 to 255
	//Each count value of of 16 bits
	//Total file Size should be 512 bytes
	char opFile[] = "hist.dat";
	char *outputFile = opFile;
	writePrepare(outputFile);
	unsigned char MSBbyte, LSBbyte;
	for (int i = 0; i <= MAX_VAL; i++) {
//		cout << P[i] << endl;
		MSBbyte = (P[i] >> 8) & 0xFF;
		writeFileByBytes(MSBbyte);
		LSBbyte = (P[i]) & 0xFF;
		writeFileByBytes(LSBbyte);
	}
	closeFile();
}

/**
 * Calculate the interval/range values
 * T = (R[i] + R[i-1])/2
 */
void calculateIntervals() {
	for (int i = 1; i < SQ::L; i++)
		SQ::T[i] = (SQ::R[i] + SQ::R[i - 1]) / 2;
}

/**
 * Recompute the values of R using the
 * probability values
 */
void getNewLevels(unsigned short *P) {
	long int num, den;
	int lower, higher;

//	printT();
	for (int i = 0; i < SQ::L; i++) {
		//upper range for lower
		lower = ceilf(SQ::T[i]);
		//If last value then include it in higher
		if (i == SQ::L - 1)
			higher = ceilf(SQ::T[i + 1]);
		else
			higher = floorf(SQ::T[i + 1]);
//		cout << i << " " << SQ::T[i] <<" "<< SQ::T[i+1] << endl;
//		cout << "Lower: " << lower << endl;
//		cout << "Higher: " << higher << endl;
		num = 0, den = 0;
		//Check the report for the formulae
		for (int j = lower; j <= higher; j++) {
			num += j * P[j];
			den += P[j];
		}
		if (den != 0) {
			SQ::R[i] = (float) num / den;
		}
	}
}

/**
 * Compute the distortion rate
 */
float getDistortionRate(float previousMSE, float currentMSE) {
	return (previousMSE - currentMSE) / currentMSE;
}

/**
 * This fucntion computes the mean square error
 * between the actual data nd quantized data.
 */
float getMSE(char *memblock) {

	double MSE = 0, diff;
	float QV;
	for (int i = 0; i < FileSizeinBytes; i++) {
		unsigned char val = memblock[i];
		QV = getQuantizedValue(val);
		diff = pow((val - QV), 2);
		MSE += diff;
	}
	MSE /= FileSizeinBytes;
	return MSE;
}

/**
 * This function gives the quantized value given
 * the key(value).
 */
float getQuantizedValue(float key) {

	//Check if it is the MAx Value
	if (key == MAX_VAL) {
		//Last position value  of SQ::R
		return SQ::R[SQ::L - 1];
	}
	int i;
	for (i = 0; i < SQ::L; i++) {
		if (key >= SQ::T[i] && key < SQ::T[i + 1])
			break;
	}

	return SQ::R[i];
}

void printT() {
	cout << "Values of the intervals (T) are : " << endl;
	for (int i = 0; i < SQ::L + 1; i++) {
		cout << SQ::T[i] << " ";
	}
	cout << endl << endl;
}

void printR() {
	cout << "Values of the centroid (R) are : " << endl;
	for (int i = 0; i < SQ::L; i++) {
		cout << SQ::R[i] << " ";
	}
	cout << endl << endl;
}
