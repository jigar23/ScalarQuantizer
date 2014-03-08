/*
 * fileIO.h
 *
 *  Created on: Jan 24, 2014
 *      Author: jigar
 */

#ifndef FILEIO_H_
#define FILEIO_H_
#include <iostream>
#include <fstream>
#include "../global.h"

//functions related to writing
void writeSingleCode(unsigned long code, char size);
void writePrepare(char *fileName);
void closeFile();
void checkStatusOfLastBit();
void writeHeader();
void WriteBit(bool x);
void writeFileByBytes(unsigned char data);

//functions related to reading
char* readFileByBytes(char filename[]);
bool checkEOF();
bool ReadBit();

int getFileSize(char *filename);

#endif /* FILEIO_H_ */
