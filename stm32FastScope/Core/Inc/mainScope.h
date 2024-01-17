/*
 * mainScope.h
 *
 *  Created on: Jun 26, 2023
 *      Author: john
 */

#ifndef INC_MAINSCOPE_H_
#define INC_MAINSCOPE_H_
#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"

#define PIXDIV 25
#define XDIV 12
#define YDIV 8

#define CLOCKTIM_PRESC 1
#define SYSCLK_FREQ 72000000
#define BUFFER_LEN (2 * PIXDIV * XDIV)
#define RISING 1
#define FALLING 0

#define BUFFER_SIZE 1024
#define FFT_BUFFER_SIZE 2048
#define LCD_WIDTH 300

void change_y_origin(uint16_t *buf);
void signal_fit(uint16_t *, uint16_t *);
void drawBackground();
void drawPlot(uint16_t *buf, uint16_t color);
void findTrigger(uint16_t *buf);
void drawTrigCursor(int16_t pos, uint16_t color);
void drawFreqCursor(int16_t pos, uint16_t color);

void fftPlot(float32_t *buf, float32_t,uint16_t color);
void fftFitPlot(uint16_t *buf);
void findFft(uint16_t *buf);

void setTimerFreq(uint32_t freq);
void handleVoltage();
void handleTime(int);
void handleScroll(int);
void handleRunMode(int);
void handleInputProbe();
void handleTrigger();
void handleTriggerLevel(int);
void handleFreqCursor(int);
void handlePlotMode();

uint8_t triggered;
uint16_t trigPoint;
uint16_t trigLevel = 70;
uint16_t freqPos = 13;
uint8_t stepDir = 0;
uint8_t trig = RISING;
uint32_t measuredFreq;
float sigPer;
float samPer;
uint32_t sampFreq = 480;
uint32_t voltPerdev = 0;


//boolean
int tbIndex = 0;
int vbIndex = 0;
int focus = 0;
int inputMode = 0;
int plotMode = 0;
int t_zoom = 0;
int trigMode = 0;

const char *t_base[] = { "50ms", "25ms", "10ms", "5ms", "2.5ms", "1ms", "0.5ms",
		"250us", "100us", "50us", "25us", "10us" };
//const char *v_base[] = { "X1", "X2", "X5.5", "X11", "X30"};
const char *vPdiv0[]= {"800mV","400mV", "140mV","72mV","26mV"};
const char *vPdiv1[] = {"10V", "5V","2V","1V","300mV"};
const char *input_probe[] = { "Pb0", "Pb1" };
const char *run_mode[] = { "x1", "x2","x3","x4","x5","x6" };  //zooming(time zooming)
const char *trigger_mode[] = { "Rise", "Fall" };
const char *plot_mode[] = { "Linear", "Log" };
const int t_base_multiplier[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000,
		2000, 5000 };

typedef struct textPos {
	int tx;
	int ty;
	uint16_t color;
};
typedef struct tdivMap {
	int baseVal;
	char *baseChar;
};

typedef struct selectionText {
	char *voltText;
	char *timeText;
	char *plotModeText;
	char *runText;
	char *triggerText;
	char *inputProbeText;
};

#endif /* INC_MAINSCOPE_H_ */
