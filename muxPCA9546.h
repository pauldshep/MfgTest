/*******************************************************************************
*   File Name: muxPCA9546.c
*  
* Description: Data and definitions for PCA9546 Switch driver
*******************************************************************************/
#ifndef __MUX_PCA9546_H__
#define __MUX_PCA9546_H__
  

#define MUX_PCA9546_I2C_ADDR    0x70

#define MUX_CHANNEL_0       0
#define MUX_CHANNEL_1       1
#define MUX_CHANNEL_2       2
#define MUX_CHANNEL_3       3

#define MUX_DIFF_PRESSURE   0   // differential pressure transducer is behind MUX channel 0
#define MUX_ABS_PRESSURE    1   // absolute pressure transducer is behind MUX channel 1

#define TOKEN_DELIMINATORS  (" ")

  
// Global Function Definitions
void    initMux(void);
void    resetMux(void);
int8_t  enableMuxOutputChannel(uint8_t channelID);
int8_t  disableMuxOutputChannel(uint8_t channelID);
uint8_t getMuxConfiguration(void);
void    resetMux(void);
void    displayMuxSerialCmdHelp(void);
void    processMuxSerialCmd(char *serCmd);
  
  
#endif  // end __MUX_PCA9546_H__

