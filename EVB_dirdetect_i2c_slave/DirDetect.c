#include <string.h>
//#include "Global.h"

#include "Driver/DrvGPIO.h"
#include "Driver/DrvAPU.h"
#include "Driver/DrvADC.h"

#include "DirDetect.h"
#include "Debug.h"

//
// Global Variables
//
INT32 direction = 0x02;

// Direction detection thread ID.
//osThreadId dirDetectThreadId;

//
// Local Variables and Defines
//

#define DIRDETECT_SIGNAL_PROCESS		(0x01)

#define PRINT_VALS 0

INT16 ai16ADC_BUF_A[ADC_BUFFER_SIZE];
INT16 ai16ADC_BUF_B[ADC_BUFFER_SIZE];
INT16 ai16ADC_BUF_C[ADC_BUFFER_SIZE];

static UINT16	s_u16Samples = 0;
static unsigned short collect_samples;
static INT16 maxOfA = 0;
//
// Local Functions
//

// Handle the direction detection ADC interrupt.
void ADC_IRQHandler()
{
	// Process the ADC direction detection interrupt.
	if (collect_samples)
	{
		if (DrvADC_GetAdcIntFlag())
		{
			ai16ADC_BUF_A[s_u16Samples] = 	DrvADC_GetConversionDataSigned(0);
			ai16ADC_BUF_B[s_u16Samples] = 	DrvADC_GetConversionDataSigned(1);
			ai16ADC_BUF_C[s_u16Samples] = 	DrvADC_GetConversionDataSigned(2);
			
			s_u16Samples++;
			
				
			if (maxOfA < ai16ADC_BUF_A[s_u16Samples])
				maxOfA = ai16ADC_BUF_A[s_u16Samples];

			if (s_u16Samples >= (ADC_BUFFER_SIZE))
			{	
				s_u16Samples = 0;
				collect_samples = 0;
			}
		}
	}
	DrvADC_ClearAdcIntFlag();

	



	// If the ADC buffers need processing, then signal the
	// direction detection thread to process the buffers.
//	if (!collect_samples)
//	{
//		// Signal the direction detection thread.
//		// buffers that were just finished with.
//		osSignalSet(dirDetectThreadId, DIRDETECT_SIGNAL_PROCESS);
//	}
}


// wait for a while for ADC value to be stable
static void SkipAdcUnstableInput(UINT16 u16SkipCount)
{
	UINT16 i;
	for(i = 0 ; i < u16SkipCount; ) 
	{ 
		if (DrvADC_GetAdcIntFlag())
		{
			DrvADC_ClearAdcIntFlag();
			i++;
		}
	}
}


//	Init ADC. Including:
//  Preamps.   Offset.   Operation Mode.
void init_ADC(void)
{															    
	DrvADC_Open();
	DrvADC_EnableAdc();

	DrvADC_EnableRegulator();
	DrvADC_SetRegulatorRC(eDRVADC_CTRS_R10K,eDRVADC_FWU_R8K);
	DrvTimer_WaitMillisecondTmr2(100);
	DrvADC_SetRegulatorRC(eDRVADC_CTRS_R600K,eDRVADC_FWU_R400K);

	DrvADC_StopConvert();

	DrvADC_PreAmpGainControl(DRVADC_PAG1_29DB, 16, DRVADC_PAG2_10DB, TRUE);


	DrvADC_SetAdcOperationMode(eDRVADC_CONTINUOUS_SCAN);		    // ap must set to eDRVADC_CONTINUOUS_SCAN for calibration
	DrvADC_SetConversionDataFormat(eDRVADC_2COMPLIMENT);		        // ap must set to DRVADC_2COMPLIMENT for calibration

	DrvADC_SetConversionSequence(eDRVADC_CH0CH1,eDRVADC_CH2CH3,eDRVADC_CH4CH5, eDRVADC_SCANEND,
									eDRVADC_SCANEND,eDRVADC_SCANEND, eDRVADC_SCANEND,eDRVADC_SCANEND);	

	DrvADC_StartConvert();
	DrvADC_AnalysisAdcCalibration();
	DrvAPU_CalibrateDacDcWithAdcDc();
	DrvADC_StopConvert();
}



// Start to record. 
void start_ADC(void)
{
	 DrvADC_StartConvert();				// start convert
	 SkipAdcUnstableInput(128);			// skip 128 * 8 samples
	 DrvADC_EnableAdcInt();				//enable ADC interrupt

	 s_u16Samples = 0;
	 collect_samples = 1;
}



void TurnOn_All(void)
{
	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_0);
	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_1);
	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_2);
//	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_3);
//	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_4);
	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_5);
	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_6);
	DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_7);
	DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_12);
	DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_13);
	DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_14);
	DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_15);
}

void TurnOff_All(void)
{
	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_0);
	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_1);
	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_2);
//	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_3);
//	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_4);
	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_5);
	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_6);
	DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_7);
	DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_12);
	DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_13);
	DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_14);
	DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_15);
}

void TurnOn_Light(INT32 i32i)
{
	switch (i32i)
	{
		case 12:
			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_0);
			direction = 6;
			break;
		case 1:
			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_1);
			direction = 7;
			break;
		case 2:
			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_2);
			direction = 8;
			break;
		case 3:
//			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_3);
			direction = 9;
			break;
		case 4:
//			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_4);
			direction = 10;
			break;
		case 5:
			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_5);
			direction = 11;
			break;
		case 6:
			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_6);
			direction = 12;
			break;
		case 7:
			DrvGPIO_ClearOutputBit(&GPIOA, DRVGPIO_PIN_7);
			direction = 1;
			break;
		case 11:
			DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_12);
			direction = 5;
			break;
		case 10:
			DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_13);
			direction = 4;
			break;
		case 9:
			DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_14);
			direction = 3;
			break;
		case 8:
			DrvGPIO_ClearOutputBit(&GPIOB, DRVGPIO_PIN_15);
			direction = 2;
			break;
	}
}

void TurnOff_Light(INT32 i32i)
{
	switch (i32i)
	{
		case 0:
			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_0); // LED 6
			direction = 6;
			break;
		case 1:
			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_1); // LED 7
			direction = 7;
			break;
		case 2:
			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_2); // LED 8
			direction = 8;
			break;
		case 3:
//			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_3); // LED 9
			direction = 9;
			break;
		case 4:
//			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_4); // LED 10
			direction = 10;
			break;
		case 5:
			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_5); // LED 11
			direction = 11;
			break;
		case 6:
			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_6); // LED 12
			direction = 12;
			break;
		case 7:
			DrvGPIO_SetOutputBit(&GPIOA, DRVGPIO_PIN_7); // LED 1
			direction = 1;
			break;
		case 11:
			DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_12); // LED 5
			direction = 5;
			break;
		case 10:
			DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_13); // LED 4
			direction = 4;
			break;
		case 9:
			DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_14); // LED 3
			direction = 3;
			break;
		case 8:
			DrvGPIO_SetOutputBit(&GPIOB, DRVGPIO_PIN_15); // LED 2
			direction = 2;
			break;
	}
}






short find_phase_AB(void)
{
	int phase,i;
	int sub = 0, cost = 0;
	int best_phase = 0, bestcost = 0x7FFFFFFF;
	
	for(phase = -PHASE_ESTIMATION_RESOLUTION; phase<PHASE_ESTIMATION_RESOLUTION; phase++) {
		cost = 0;
		for(i = PHASE_ESTIMATION_RESOLUTION; i<(ADC_BUFFER_SIZE - PHASE_ESTIMATION_RESOLUTION); i++) {
			sub = ai16ADC_BUF_A[i + phase] - ai16ADC_BUF_B[i];
			cost = cost + sub*sub;
		}
		if (bestcost > cost) {
			bestcost = cost;
			best_phase = phase;
		}
	}
	return best_phase;
}

short find_phase_AC(void)
{
	int phase,i;
	int sub = 0, cost = 0;
	int best_phase = 0, bestcost = 0x7FFFFFFF;
	
	for(phase = -P_E_RES; phase<P_E_RES; phase++) {
		cost = 0;
		for(i = P_E_RES; i<ADC_BUFFER_SIZE - P_E_RES; i++) {
			sub = ai16ADC_BUF_A[i + phase] - ai16ADC_BUF_C[i];
			cost = cost + sub*sub;
		}
		if (bestcost > cost) {
			bestcost = cost;
			best_phase = phase;
		}
	}
	return best_phase;
}


short find_phase_BC(void)
{
	int phase,i;
	int sub = 0, cost = 0;
	int best_phase = 0, bestcost = 0x7FFFFFFF;
	
	for(phase = -P_E_RES; phase<P_E_RES; phase++) {
		cost = 0;
		for(i = P_E_RES; i<ADC_BUFFER_SIZE - P_E_RES; i++) {
			sub = ai16ADC_BUF_B[i + phase] - ai16ADC_BUF_C[i];
			cost = cost + sub*sub;
		}
		if (bestcost > cost) {
			bestcost = cost;
			best_phase = phase;
		}
	}

	return best_phase; 
}




void determineDirection(short phaseAB, short phaseAC, short phaseBC)
{
	static INT32 light, prev_light = 0;
	static short consistency_counter = 1;
	short size = 10;
	
	prev_light = light;

	if ((phaseAB <= size) && (phaseAB >= -size)) {
		if ((phaseAC > 0) && (phaseBC > 0))
			light = 2;
		else
			light = 8;
	}
	else if ((phaseAC <= size) && (phaseAC >= -size)) {
		if ((phaseAB > 0) && (phaseBC < 0))
			light = 10;
		else
			light = 4;
	}
	else if ((phaseBC <= size) && (phaseBC >= -size)) {
		if ((phaseAC < 0) && (phaseAB < 0))
			light = 6;
		else
			light = 12;
	}
	else if ((phaseAB < 0) && (phaseAC < 0)) {  // sound hits A first
		if (phaseBC > 0)
			light = 5;
		else
			light = 7;
	}
	else if ((phaseAC > 0) && (phaseBC > 0)) {  // sound hits C first
		if (phaseAB > 0)
			light = 1;
		else
			light = 3;
	}
	else if ((phaseAB > 0) && (phaseBC < 0)) {  // sound hits B first
		if (phaseAC > 0)
			light = 11;
		else
			light = 9;
	}
	else {
		TurnOff_All();
		consistency_counter = 1;
		return;
	}
	
	// only turn on light if we are very confident in that direction
	if (light == prev_light)
		consistency_counter++;
	else
		consistency_counter = 1;
	
	if (consistency_counter >=2) {
		consistency_counter = 1;
		TurnOff_All();
		TurnOn_Light(light);
	}
}

int compute_average_sound_level(int currentSoundLevel) {
	static int average_sound_level = 0;
	static int count;
	
	// IIR filter
	if (count >=10) {
		average_sound_level = average_sound_level*99 + currentSoundLevel*1;
		average_sound_level /= 100; 
		count = 0;
	}
	count++;
//	average_sound_level = currentSoundLevel;
	
	// bump it up a bit so only louder sounds are heard
	//average_sound_level *= 1.2;
	return average_sound_level;
}

void Do_Loop(void)
{
	static int phaseAB, phaseAC, phaseBC;
	uint32_t multiplier = 10;
	static int avgSoundLevel;
	
//	int k;

	if (!collect_samples) { //if we have collected an array's worth of data (and therefore not currently collecting samples), then do this interpretation.
		
//		avgSoundLevel = 1;
		avgSoundLevel = compute_average_sound_level(maxOfA*10);
//		avgSoundLevel = avgSoundLevel*9 + maxOfA*10;
//		avgSoundLevel /= 10; 
		
		// ignore sounds that are too soft and just start sampling again
		if((maxOfA*10 < avgSoundLevel*1.2) || (maxOfA < 40)) {
			maxOfA = 0;
			collect_samples = 1;
			return;
		}

	 
		// estimate phase
//		phaseAB = find_phase_AB()*multiplier*2;
//		phaseAC = find_phase_AC()*multiplier;
//		phaseBC = find_phase_BC()*multiplier;
		
		phaseAB = phaseAB*8 + find_phase_AB()*2*multiplier;
		phaseAC = phaseAC*8 + find_phase_AC()*2*multiplier;
		phaseBC = phaseBC*8 + find_phase_BC()*2*multiplier;

		phaseAB /= 10;
		phaseAC /= 10;
		phaseBC /= 10;
		// start adc conversions again
		
		collect_samples = 1; // let the ADC_ISR take data again
		
		
		
		// determine direction
		determineDirection(phaseAB, phaseAC, phaseBC);
		maxOfA = 0;
	}
}

// Process the direction detection buffers.
//static void dirDetectProcessBuffers(void)
//{
//	// turn off ADC interrupt
//	collect_samples = 0;
//	
//	// XXX Process the direction detection buffers here.
//	Do_Loop();
//}






// Direction detection processing thread.
//void dirDetectThread(void const *argument)
//{
//	// XXX Initialize ADC hardware to sample microphones.

//	// We loop forever processing direction detection events.
//	for (;;)
//	{
//		// Wait for a signal indicating direction buffers should be processed.
//		osEvent evt = osSignalWait(0, osWaitForever);

//		// Make sure a signal was detected.
//		if (evt.status != osEventSignal) continue;
//	
//		// Process direction detection signal.
//		if (evt.value.signals & DIRDETECT_SIGNAL_PROCESS) dirDetectProcessBuffers();
//	}
//}

//
// Global Functions
//



// enable direction detection
void dirDetectInit(void) {
	init_ADC();
	start_ADC();
	for (;;) {
		Do_Loop();
	}
}

//void signalSettingFunction(void) {
//	osSignalSet(dirDetectThreadId, DIRDETECT_SIGNAL_PROCESS);
//}
