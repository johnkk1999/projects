/*
 * fft.h
 *
 *  Created on: Jul 15, 2023
 *      Author: john
 */

#ifndef INC_FFT_H_
#define INC_FFT_H_
//#include "arm_math.h"
//#include "arm_const_structs.h"

#define SAMPLES 1024
#define FFT_LENGTH 512
#define FFT_MAXVALUE 100

//uint16_t fftOutput[FFT_LENGTH/2];
//float32_t fftInput[SAMPLES];
//
//TM_FFT_F32_t FFT;
void initFft();
void findFft(uint16_t *buf);


#endif /* INC_FFT_H_ */
