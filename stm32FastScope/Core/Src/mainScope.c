/*
 * mainScope.c
 *
 *  Created on: Jun 26, 2023
 *      Author: john kariuki
 *
 */
#include "main.h"
#include "mainScope.h"
#include <stdlib.h>
#include "Lcd/lcd.h"
#include "Lcd/stm32_adafruit_lcd.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart1;

volatile uint16_t convComplete = 0;
int y_space = 20;
int x_space = 10;
int w = 0;
int h = 0;
int wb = 0;
int hb = 0;
volatile int check = 5;
int selIndex[6];
volatile int buf_start = 0;
volatile uint32_t currentMillis = 0;
volatile uint32_t previousMillis = 0;

static uint16_t buf[BUFFER_SIZE];
//static uint16_t bufp[BUFFER_SIZE];
static uint16_t fftBufIn[FFT_BUFFER_SIZE];

struct textPos tp[6];
struct selectionText sText;
void creatSelection(struct selectionText);
int fft_ifft = 0;

void mainScope() {
	BSP_LCD_Init();
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	//initial opamp gain
	HAL_GPIO_WritePin(GPIOA, mux_A_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, mux_B_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, mux_C_Pin, GPIO_PIN_SET);
	//initialising the selection options
	sText.timeText = t_base[0];
	sText.voltText = vPdiv0[0];
	sText.plotModeText = plot_mode[0];
	sText.runText = run_mode[0];
	sText.triggerText = trigger_mode[0];
	sText.inputProbeText = input_probe[0];

	creatSelection(sText);
	drawBackground();
	wb = 300;
	hb = 200;
	setTimerFreq(480);
	//initialising fft
//	arm_cfft_radix4_instance_f32 S;

//setTimerFreq(20000);
// loop
	while (1) {

		//BSP_LCD_Clear(LCD_COLOR_DARKBLUE);

		/*
		 HAL_ADC_Start(&hadc1);
		 for (uint32_t i = 0; i < 300; i++) {
		 buf[i] = HAL_ADC_GetValue(&hadc1);
		 }
		 HAL_ADC_Stop(&hadc1);

		 */
		//starting adc using DMA mode
//fftMode
		static float32_t compBuf[FFT_BUFFER_SIZE * 2];
		static float32_t realBuf[FFT_BUFFER_SIZE];
//		static float32_t realBufPrev[FFT_BUFFER_SIZE];
		uint32_t peakHz;

		HAL_ADC_Start_DMA(&hadc1, (uint32_t*) fftBufIn, FFT_BUFFER_SIZE);
		HAL_TIM_Base_Start(&htim3);
		while (!convComplete)
			;
		HAL_TIM_Base_Stop(&htim3);
		convComplete = 0;

		//normal mode
//
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_FillRect(x_space, y_space, wb, hb / 2);
		signal_fit(fftBufIn, buf);
		//	change_y_origin(buf);

		findTrigger(buf);
//		drawPlot(bufp, LCD_COLOR_BLACK);
		drawBackground();
		drawPlot(buf, LCD_COLOR_GREEN);

		//performing fft
//
		for (int i = 0; i < FFT_BUFFER_SIZE; i++) {
			compBuf[(uint16_t) (2 * i)] = (float32_t) ((float32_t) fftBufIn[i]
					- 128);
			compBuf[(uint16_t) (i * 2 + 1)] = 0.0f;
		}
		float32_t maxValue;
		uint32_t maxIndex;
//			arm_cfft_radix4_init_f32(&S, FFT_BUFFER_SIZE,0,1);
//			arm_cfft_radix4_f32(&S,compBuf);
//			arm_cmplx_mag_f32(compBuf,realBuf,FFT_BUFFER_SIZE);
//			arm_max_f32(realBuf,FFT_BUFFER_SIZE,&maxValue, &maxIndex);
		//fftPlot(realBufPrev, maxValue, LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		BSP_LCD_FillRect(x_space, 120, wb, hb / 2);

		arm_cfft_f32(&arm_cfft_sR_f32_len2048, (float32_t*) compBuf, 0, 1);
		arm_cmplx_mag_f32((float32_t*) compBuf, realBuf, FFT_BUFFER_SIZE);
		arm_max_f32((float32_t*) realBuf, FFT_BUFFER_SIZE, &maxValue,
				&maxIndex);
		if (focus != 7) {
			peakHz = (uint32_t) (((float32_t) (sampFreq * maxIndex))
					/ (float32_t) FFT_BUFFER_SIZE);

			char pHz[15];
			sprintf(pHz, "Freq:%d", peakHz);
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_FillRect(10, 2, 100, 15);
			BSP_LCD_SetFont(&Font12); // 8 x_pixe per char
			BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
			BSP_LCD_DisplayStringAt(10, 2, pHz, LEFT_MODE);
		}
//		HAL_UART_Transmit_DMA(&huart1, (uint8_t*) pHz,
//								strlen(pHz));
		drawBackground();
		fftPlot(realBuf, maxValue, LCD_COLOR_WHITE);

//		for (uint16_t i = trigPoint; i < LCD_WIDTH + trigPoint; i++) {
//			bufp[i] = buf[i];
//		}

//			peakHz = (uint32_t)(((float32_t)(sampFreq * maxIndex) )/ (float32_t) FFT_BUFFER_SIZE);
//				char user_data[30];
//				sprintf(user_data, "%d\n",(int)maxIndex);
//				HAL_UART_Transmit_DMA(&huart1, (uint8_t*) user_data,
//						strlen(user_data));

	}

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
	convComplete = 1;
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	currentMillis = HAL_GetTick();

	if (currentMillis - previousMillis > 40) {		//removes button debouncing
		switch (GPIO_Pin) {
		case select_Pin:
			if (focus < 7)
				focus += 1;
			else
				focus = 0;

			creatSelection(sText);
			break;
		case ok_Pin:
			if (focus > 0)
				focus -= 1;
			else
				focus = 7;
			creatSelection(sText);
			break;

		case inc_Pin:
			handleScroll(0);
			break;
		case dec_Pin:
			handleScroll(1);
			break;
		default:
			break;
		}
		previousMillis = currentMillis;
	}

}
void signal_fit(uint16_t *buf, uint16_t *buf2) {
	for (int i = 0; i < FFT_BUFFER_SIZE; i++) {
		buf2[i] = 120 - buf[i] * 100 / 255;
//		buf[i] = (uint16_t) (2.0 * ((3.3 * (double) buf[i]) / 255.0));
	}
}
//void change_y_origin(uint16_t *buf) {
//
//	for (int i = 0; i < BUFFER_SIZE; i++) {
//		buf[i] = 220 - buf[i];
//	}
//}

void fftPlot(float32_t *buf, float32_t maxVal, uint16_t color) {
	BSP_LCD_SetTextColor(color);
	for (int i = 0, k = x_space; i < FFT_BUFFER_SIZE / 2; i++, k++) {
//		BSP_LCD_DrawLine((uint16_t) (10.0 + i * 300 / 512), 120,
//				(uint16_t) (10.0 + i * 300 / 512), 100 - buf[i] * 100 / maxVal);
		BSP_LCD_DrawVLine((uint16_t) (10.0 + i * 300 / (FFT_BUFFER_SIZE / 2)),
				215 - (uint16_t) (buf[i] * 90.0 / maxVal),
				(uint16_t) (buf[i] * 90.0 / maxVal));

	}
	focus == 7 ?
			drawFreqCursor(freqPos, LCD_COLOR_BLUE) :
			drawFreqCursor(freqPos, LCD_COLOR_CYAN);
}

void drawPlot(uint16_t *buf, uint16_t color) {

	BSP_LCD_SetTextColor(color);

	int k = 0;
	for (int i = trigPoint; i < trigPoint + (LCD_WIDTH / (t_zoom + 1)); i++) {

		BSP_LCD_DrawLine(x_space + (t_zoom + 1) * k, buf[i],
				x_space + (t_zoom + 1) * (k + 1), buf[i + 1]);

//		//
//		k = k + 20;

		k ++;

	}

}

void drawBackground() {
	BSP_LCD_SetFont(&Font8);
	BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
	BSP_LCD_DrawRect(x_space, y_space, wb, hb);
	BSP_LCD_DrawHLine(x_space, 120, wb);

	BSP_LCD_SetTextColor(LCD_COLOR_GRAY);
	//horizontal grid
	for (uint16_t i = y_space + 25; i < hb / 2; i += 25) {
		if (i == 70) {
			BSP_LCD_DrawHLine(x_space, i, wb);
		} else
			for (uint16_t j = 0; j < wb; j += 10) {

				BSP_LCD_DrawHLine(x_space + j, i, 5);
			}

	}
	for (uint16_t i = x_space + 25; i < wb; i += 25) {
		for (uint16_t j = 0; j < hb / 2; j += 10) {
			BSP_LCD_DrawVLine(i, y_space + j, 5);
		}

	}

}
void setTimerFreq(uint32_t freq) {

	uint16_t arr = (SYSCLK_FREQ / ((CLOCKTIM_PRESC + 1) * freq)) - 1;
	htim3.Instance->ARR = arr;

}
void drawTrigCursor(int16_t pos, uint16_t color) {
	Point cp[5];
	cp[0].X = 10;
	cp[0].Y = pos;
	cp[1].X = 5;
	cp[1].Y = pos - 3;
	cp[2].X = 0;
	cp[2].Y = pos - 3;
	cp[3].X = 0;
	cp[3].Y = pos + 3;
	cp[4].X = 5;
	cp[4].Y = pos + 3;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(0, trigLevel - 10, 10, 20);
	//BSP_LCD_FillRect(0,trigLevel - 3,10,6);
	BSP_LCD_SetTextColor(color);
	BSP_LCD_FillPolygon(&cp[0], 5);

}

void drawFreqCursor(int16_t pos, uint16_t color) {
	Point cp[5];
	cp[0].X = pos;
	cp[0].Y = 126;
	cp[1].X = pos + 3;
	cp[1].Y = 123;
	cp[2].X = pos + 3;
	cp[2].Y = 120;
	cp[3].X = pos - 3;
	cp[3].Y = 120;
	cp[4].X = pos - 3;
	cp[4].Y = 123;
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	if (stepDir == 1) {
		BSP_LCD_FillRect(pos - 3, 120, 6, 6);
		BSP_LCD_DrawVLine(pos - 3, 126, 94);
	} else {
		BSP_LCD_FillRect(pos + 3, 120, 6, 6);
		BSP_LCD_DrawVLine(pos + 3, 126, 94);
	}
	//BSP_LCD_FillRect(0,trigLevel - 3,10,6);
	BSP_LCD_SetTextColor(color);
	BSP_LCD_FillPolygon(&cp[0], 5);
	BSP_LCD_DrawVLine(pos, 126, 94);
}

void findTrigger(uint16_t *buf) {
	trigPoint = 0;
	uint16_t trigPoint2 = 0;
	triggered = 0;
	measuredFreq = 0;

	for (int i = 1; i < BUFFER_SIZE / 2 && triggered != 2; i++) {
		if ((trig == RISING && buf[i] >= trigLevel && buf[i - 1] < trigLevel)
				|| (trig == FALLING && buf[i] <= trigLevel
						&& buf[i - 1] > trigLevel)) {
			if (!triggered) {
				trigPoint = i;
				triggered = 1;
			} else {
				trigPoint2 = i;
				triggered = 2;
			}
		}
		if (triggered == 2) {
			measuredFreq = (uint32_t) ((float32_t) sampFreq
					/ (float32_t) (trigPoint2 - trigPoint));
			//measuredFreq = 84000000.0 / sigPer;
		}
	}
}

void creatSelection(struct selectionText tx) {

	for (int i = 0; i < 6; i++)

	{

		int x = x_space + (51 * i);
		int y = 242 - y_space;
		tp[i].tx = x + 2;
		tp[i].ty = y + 2;
		if (i == focus) {
			tp[i].color = LCD_COLOR_BLUE;
			BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
			BSP_LCD_FillRect(x, y, 50, 14);

		}

		else {
			tp[i].color = LCD_COLOR_BLACK;
			BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
			BSP_LCD_FillRect(x, y, 50, 14);
		}
	}

	focus == 6 ?
			drawTrigCursor(trigLevel, LCD_COLOR_BLUE) :
			drawTrigCursor(trigLevel, LCD_COLOR_CYAN);
	focus == 7 ?
			drawFreqCursor(freqPos, LCD_COLOR_BLUE) :
			drawFreqCursor(freqPos, LCD_COLOR_CYAN);
//	if (focus == 6)
//		drawTrigCursor(trigLevel, LCD_COLOR_BLUE);
//	else
//		drawTrigCursor(trigLevel, LCD_COLOR_CYAN);

	BSP_LCD_SetFont(&Font12); // 8 x_pixe per char
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

	//input probe divider
	BSP_LCD_SetBackColor(tp[0].color);
	BSP_LCD_DisplayStringAt(tp[0].tx, tp[0].ty, tx.inputProbeText, LEFT_MODE);
// voltage per division
	BSP_LCD_SetBackColor(tp[1].color);
	BSP_LCD_DisplayStringAt(tp[1].tx, tp[1].ty, tx.voltText, LEFT_MODE);
//time per division
//char t_div[7] = "200uS  ";
	BSP_LCD_SetBackColor(tp[2].color);
	BSP_LCD_DisplayStringAt(tp[2].tx, tp[2].ty, tx.timeText, LEFT_MODE);
	//run or stop mode
	BSP_LCD_SetBackColor(tp[3].color);
	BSP_LCD_DisplayStringAt(tp[3].tx, tp[3].ty, tx.runText, LEFT_MODE);
//Log or  Linear
	BSP_LCD_SetBackColor(tp[4].color);
	BSP_LCD_DisplayStringAt(tp[4].tx, tp[4].ty, tx.plotModeText, LEFT_MODE);

//Rising or falling Edge
	BSP_LCD_SetBackColor(tp[5].color);
	BSP_LCD_DisplayStringAt(tp[5].tx, tp[5].ty, tx.triggerText, LEFT_MODE);

}
void handleVoltage(int mode) {

	if (mode == 0) {
		if (vbIndex < 4) { //reducing the voltage base
			vbIndex += 1;
			sText.voltText = inputMode == 0 ? vPdiv0[vbIndex] : vPdiv1[vbIndex];
		} else {
			vbIndex = 0;
			sText.voltText = inputMode == 0 ? vPdiv0[vbIndex] : vPdiv1[vbIndex];
//			sText.voltText = v_base[vbIndex];
		}
	} else if (mode == 1) {
		if (vbIndex > 0) { //for increasing the voltage base
			vbIndex -= 1;
			sText.voltText = inputMode == 0 ? vPdiv0[vbIndex] : vPdiv1[vbIndex];
//			sText.voltText = v_base[vbIndex];

		} else {
			vbIndex = 4;
			sText.voltText = inputMode == 0 ? vPdiv0[vbIndex] : vPdiv1[vbIndex];
		}
	} else
		sText.voltText = inputMode == 0 ? vPdiv0[vbIndex] : vPdiv1[vbIndex];
	//Setting gain
	switch (vbIndex) {
	case 0:
		HAL_GPIO_WritePin(GPIOA, mux_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_B_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, mux_C_Pin, GPIO_PIN_SET);
		break;
	case 1:
		HAL_GPIO_WritePin(GPIOA, mux_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_C_Pin, GPIO_PIN_SET);
		break;
	case 2:
		HAL_GPIO_WritePin(GPIOA, mux_A_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_C_Pin, GPIO_PIN_RESET);

		break;
	case 3:
		HAL_GPIO_WritePin(GPIOA, mux_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, mux_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_C_Pin, GPIO_PIN_SET);
		break;
	case 4:
		HAL_GPIO_WritePin(GPIOA, mux_A_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, mux_B_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, mux_C_Pin, GPIO_PIN_RESET);
		break;
	default:
		break;
	}
	// Update menu
	creatSelection(sText);
}

void handleTime(int mode) {
	if (mode == 0) {
		if (tbIndex < 11) { //reducing the time base
			tbIndex += 1;
			sText.timeText = t_base[tbIndex];

		} else {
			tbIndex = 0;
			sText.timeText = t_base[tbIndex];

		}
	} else {
		if (tbIndex > 0) { //for increasing the time base
			tbIndex -= 1;
			sText.timeText = t_base[tbIndex];
		} else {
			tbIndex = 11;
			sText.timeText = t_base[tbIndex];
		}
	}
	sampFreq = 480 * t_base_multiplier[tbIndex];
	setTimerFreq(sampFreq);
	creatSelection(sText);

}
void handleScroll(int direction) {
	switch (focus) {
	case 0:
		handleInputProbe();

		break;
	case 1:
		handleVoltage(direction);
		break;
	case 2:
		handleTime(direction);
		break;
	case 3:
		handleRunMode(direction);

		break;
	case 4:
		handlePlotMode();
		break;
	case 5:
		handleTrigger();
		break;
	case 6:
		handleTriggerLevel(direction);
		break;
	case 7:
		handleFreqCursor(direction);
		break;
	default:
		break;
	}
}

void handleRunMode(int direction) {

	if (direction == 0) {
		if (t_zoom < 5) {
			t_zoom += 1;
			sText.runText = run_mode[t_zoom];
		} else {
			t_zoom = 0;
			sText.runText = run_mode[t_zoom];
		}
	} else {
		if (t_zoom > 0) {
			t_zoom -= 1;
			sText.runText = run_mode[t_zoom];
		} else {
			t_zoom = 5;
			sText.runText = run_mode[t_zoom];
		}
	}

	creatSelection(sText);

}
void handleInputProbe() {

	if (inputMode == 0) {
		inputMode = 1;
		sText.inputProbeText = input_probe[0];
	}

	else {
		inputMode = 0;
		sText.inputProbeText = input_probe[1];
	}
	handleVoltage(2);
	creatSelection(sText);
}
void handleTrigger() {

	if (trigMode == 0) {
		trigMode = 1;
		sText.triggerText = trigger_mode[0];
	} else {
		trigMode = 0;
		sText.triggerText = trigger_mode[1];
	}

}
void handleFreqCursor(int direction) {
	if (direction == 0 && freqPos < 307) {
		freqPos += 2;
		stepDir = 1;
	} else if (direction == 1 && freqPos > 13) {
		freqPos -= 2;
		stepDir = 0;
	}
	uint32_t freq = (freqPos - 13)
			* (uint32_t) ((float32_t) sampFreq / (float32_t) 600);
	char pHz[15];
	sprintf(pHz, "Freq:%d", freq);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_FillRect(10, 2, 100, 15);
	BSP_LCD_SetFont(&Font12); // 8 x_pixe per char
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(10, 2, pHz, LEFT_MODE);
	creatSelection(sText);
}
void handleTriggerLevel(int direction) {
	if (direction == 0 && trigLevel < 120) {
		trigLevel += 2;
	} else if (direction == 1 && trigLevel > 20) {
		trigLevel -= 2;
	}

	creatSelection(sText);
}
void handlePlotMode() {
	if (plotMode == 0) {
		plotMode = 1;
		sText.plotModeText = plot_mode[0];

	} else {
		plotMode = 0;
		sText.plotModeText = plot_mode[1];
	}
	creatSelection(sText);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
// write some code when transmission is complete
}
