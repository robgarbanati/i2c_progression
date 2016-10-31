#include "cmsis_os.h"
#include "Driver/DrvADC.h"
#include "Driver/DrvCLK.h"
#include "Driver/DrvSYS.h"
#include "Driver/DrvGPIO.h"
#include "Driver/DrvTimer.h"
#include "Adc.h"

// Contains the most recent microphone sample.
static INT16 adcSampleMicrophone;

//
// Local Functions
//

void ADC_IRQHandler()
{
	// Clear the ADC interrupt flag.
	DrvADC_ClearAdcIntFlag();

	// Update each of the samples.
	adcSampleMicrophone = DrvADC_GetConversionDataSigned(0);
}

static void adcDCCalibration()
{
	// Make sure any conversion is stopped.
	DrvADC_StopConvert();

	// Set Amp Gain Control to what it needs.
	DrvADC_PreAmpGainControl(ADC_PAG_GAIN1, 0, ADC_PAG_GAIN2, TRUE);

	// Set to eDRVADC_CONTINUOUS_SCAN for calibration.
	DrvADC_SetAdcOperationMode(eDRVADC_CONTINUOUS_SCAN);

	// Must set to DRVADC_2COMPLIMENT for calibration.
	DrvADC_SetConversionDataFormat(eDRVADC_2COMPLIMENT);

	// Set the calibration conversion sequence.  Cannot be 1xx1/1110 for calibration.
	DrvADC_SetConversionSequence(eDRVADC_CH0CH1,eDRVADC_CH0CH1,eDRVADC_CH0CH1,eDRVADC_CH0CH1,
															 eDRVADC_CH0CH1,eDRVADC_CH0CH1,eDRVADC_CH0CH1,eDRVADC_CH0CH1);	

	// Use software to calibrate offset bias.
	DrvADC_StartConvert();
	DrvADC_AnalysisAdcCalibration();
	DrvADC_StopConvert();
}

//
// Global Functions
//

void adcInit(void)
{
	// Open the ADC engine clock.
 	DrvADC_Open();

	// Enable the ADC IP.
	DrvADC_EnableAdc();

#if (defined(__N572F072__) || defined(__N572P072__)) 
	// Select RC value for regulator fast stable.
	DrvADC_SetRegulatorRC(eDRVADC_CTRS_R10K, eDRVADC_FWU_R8K);

	// Enable regulator for PGC power.
	DrvADC_EnableRegulator();

	// Wait for regulator stable.
	osDelay(100);

	// Reset RC value for normal PGC setting.
	DrvADC_SetRegulatorRC(eDRVADC_CTRS_R600K, eDRVADC_FWU_R400K);
#endif

	// Calibrate offset bias by ADC software.
	adcDCCalibration();

	// Set the single conversion operation mode.
	DrvADC_SetAdcOperationMode(eDRVADC_CONTINUOUS_SCAN);

	// Set the conversion data format.
	DrvADC_SetConversionDataFormat(eDRVADC_2COMPLIMENT);

	// Set the conversion sequence for channels 0 and 1.
	DrvADC_SetConversionSequence(eDRVADC_CH0CH1, eDRVADC_SCANEND, eDRVADC_SCANEND, eDRVADC_SCANEND,
															 eDRVADC_SCANEND, eDRVADC_SCANEND, eDRVADC_SCANEND, eDRVADC_SCANEND);

	// Enable ADC interrupt.
	DrvADC_EnableAdcInt();

	// Start continuous conversions.
	DrvADC_StartConvert();
}

INT16 adcGetMicrophone(void)
{
	INT16 result;

	// Disable interrupts.
	__disable_irq(); 

	// Get the sample.
	result = adcSampleMicrophone;

	// Enable interrupts.
	__enable_irq(); 

	return result;
}

