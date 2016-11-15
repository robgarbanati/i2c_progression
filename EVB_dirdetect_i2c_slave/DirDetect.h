#ifndef __DIRDETECT_H
#define __DIRDETECT_H

//
// Global Defines and Declarations
//
#define SPEED_OF_SOUND 340290 // in mm/s
#define DISTANCE_BETWEEN_MICS 65 // in mm
#define SOUND_TRAVEL_FREQUENCY (SPEED_OF_SOUND/DISTANCE_BETWEEN_MICS + 1) // 1/(amount of time it take sound to travel from one speaker to another)
#define PHASE_ESTIMATION_RESOLUTION 9  //  minimum number of points needed to sample while sound is travelling from one mic to another (in the longest case) to get useful phase estimates
#define P_E_RES PHASE_ESTIMATION_RESOLUTION // easier to read
#define SAMPLE_FREQUENCY (SOUND_TRAVEL_FREQUENCY*P_E_RES) // necessary sampling frequency to get our phase_estimation_resolution
#define CYCLES_PER_CONVERSION 25
#define NUM_CHANNELS 3
#define ADC_CLOCK_FREQUENCY (SAMPLE_FREQUENCY * CYCLES_PER_CONVERSION * NUM_CHANNELS) // necessary clock rate


#define NUM_STF_WAVES_PER_BUFFER 7 // STF = SOUND_TRAVEL_FREQUENCY
#define ADC_BUFFER_SIZE (NUM_STF_WAVES_PER_BUFFER*PHASE_ESTIMATION_RESOLUTION)

//
// Global Functions
//

// Init
void dirDetectInit(void);

#endif // __DIRDETECT_H


