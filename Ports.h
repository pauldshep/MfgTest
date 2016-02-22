/******************************************************************************
*        File: Ports.h
*
* Description: ATMega128 port data and definitions:
*
*              Pin  Port Bit Dir Int Assignment
*
*                   A    0           (AD0) Coil 2A Control  NOTE: external memory interface address
*                   A    1           (AD1) Coil 2B Control
*                   A    2           (AD2) Coil 1A Control
*                   A    3   in  3   (AD3) Coil 1B Control / (switch - Enter ???)
*                   A    4   in  6   (AD4) switch - Up
*                   A    5   in  7   (AD5) switch - ESC
*                   A    6   in  2   (AD6) switch - Down
*                   A    7   in  5   (AD7) switch - Pump
*
*                   B    0   out     (SS)   /SPI_SS                  -- SPI Bus, LED
*              11   B    1   out     (SCK)  SPI_SCK                  -- VNC2_SPI_SCLK 
*                   B    2   out     (MOSI) SPI_MOSI 
*                   B    3   in      (MISO) SPI_MISO
*                   B    4   out     (OC0)  UART_CTS                 -- VNC2_IOBUS07
*                   B    5   out     (OC1A) PWM Moderator Control
*                   B    6   out     (OC1B) PWM Nozzle Control
*                   B    7   out     (OC1C) PWM Sample Control
*
*                   C    0   out     (A8)  Water Injection Pump Control
*                   C    1           (A9)  Quadrature A
*                   C    2           (A10) Quadrature B
*                   C    3           (A11) Installed
*                   C    4   out     (A12) Sample Air Pump Control
*                   C    5           (A13) SD_RDY
*                   C    6           (A14) -
*                   C    7           (A15) -
*
*                   D    0           (INT0) SCL -- I2C Bus
*                   D    1           (INT1) SDA -- I2C Bus
*                   D    2           (INT2/RXD1)
*                   D    3           (INT3/TXD1) (Switch Enter ???)
*                   D    4           (ICP1) - I2C Switch /Reset      
*                   D    5           (XCK1)
*                   D    6           (TI)
*                   D    7           (T2)
*           
*              02   E    0  in       (RXD0/PDI)  ISP_PDI, UART_TX    -- VNC2_SPI_MOSI -- VNC2_IOBUS04
*              03   E    1  out      (TXD0/PDO)  ISP_PDO, UART_RX    -- VNC2_SPI_MISO -- VNC2_IOBUS05
*                   E    2  in       (XCK0/AIN0) UART_RTS                             -- VNC2_IOBUS06
*                   E    3  out      (OC3A/AIN1) PWM Initiator Control
*                   E    4  out      (OC3B/INT4) PWM Conditioner Control
*                   E    5  in  5    (OC3C/INT5) PWM Sample Pump 
*                   E    6  in  6    (T3/INT6)   Sample Pump Encoder Index 
*                   E    7           (ICP3/INT7) Int Key
*
*                   F    0           (ADC0) Conditioner Temp
*                   F    1           (ADC1) Initiator Temp
*                   F    2           (ADC2) Moderator Temp
*                   F    3           (ADC3) Nozzle Temp
*              57   F    4           (ADC4/TCK) Sample Temp          -- JTAG
*              56   F    5           (ADC5/TMS) Case Temp            -- JTAG
*              55   F    6           (ADC6/TDO) Spare Thermistor 1   -- JTAG
*              54   F    7           (ADC7/TDI) Spare Thermistor 2   -- JTAG
*
*                   G    0           (WR)
*                   G    1           (RD)
*                   G    2           (ALE)
*
*              Pin
*
*              01       in           (PEN)/ISP_RESET                 -- VNC2_SPI_SS
*              20                    Reset                           -- ISP         
* 
* 
*              I2C Addresses:
*              Addr     Sensor
* 
*              0x28     NPA700 Pressure Sensors(2) - multiplex to switch between
*              0x29     Zepher FlowMeter - HAFBLF200C2AX5
*              0x49     Zepher FlowMeter - HAFUHM0010L4AXT
*              0x50
*              0x68     DS1307 Real Time Clock (Dallas Semiconductor)
*              0x70     TI PCA9546A Switch With Reset (PD4)
*              0x??     ChipCap Humidity and Temperature Sensor (Amphenol)
* 
* 
*              PWM (two 8-bit PWM Channels and six 16-bit):
* 
*              Port Function        Description
* 
*              B5   (OC1A)          PWM Moderator Control   - Output Compare and PWM Output A for Timer/Counter1
*              B6   (OC1B)          PWM Nozzle Control      - Output Compare and PWM Output B for Timer/Counter1
*              B7   (OC1C)          PWM Sample Control      - Output Compare and PWM Output for Timer/Counter2 or 
*                                                             Output Compare and PWM Output C for Timer/Counter1 
* 
*              E3   (OC3A/AIN1)     PWM Initiator Control   - Output Compare and PWM Output A for Timer/Counter3
*              E4   (OC3B/INT4)     PWM Conditioner Control - Output Compare and PWM Output B for Timer/Counter3
*              E5   (OC3C/INT5)     PWM Sample Pump         - Output Compare and PWM Output C for Timer/Counter3
* 
*******************************************************************************/
#ifndef __Ports_h__
#define __Ports_h__

//////////////////////////////////////////////
////
//// Input Ports:
////
//////////////////////////////////////////////


#define ENCODER_A         _BV(PE6)  // interrupt 5
#define ENCODER_INDEX     _BV(PE5)	// interrupt 6
#define ENCODER_DDR	      DDRE
#define ENCODER_INPUTS	 ( ENCODER_A | ENCODER_INDEX )

#define ENCODER_PORT PORTE
// Switches

#define SW_PUMP   _BV(PA7)    // interrupt 5
#define SW_DWN    _BV(PA6)    // interrupt 2
#define SW_ESC    _BV(PA5)    // interrupt 7
#define SW_UP     _BV(PA4)	  // interrupt 6
#define SW_ENTER  _BV(PD3)    // interrupt 3

#define SWITCH_DDR1  DDRA
#define SWITCH_DDR2  DDRD

#define SWITCH_INPUTS1  ( SW_DWN | SW_PUMP | SW_UP | SW_ESC )                   // SWITCH_DDR1 &= ~SWITCH_INPUTS1   makes pins inputs
#define SWITCH_INPUTS2  ( SW_ENTER )        // SWITCH_DDR2 &= ~SWITCH_INPUTS2   makes pins inputs


#define SWITCH_INPUTS  ((PINA & ( SW_DWN | SW_UP | SW_ESC | SW_PUMP ))  | (PIND & SW_ENTER))

#define		ENTER_PRESSED	(SW_DWN | SW_UP  | SW_ESC  | SW_PUMP  )    // switches active low!
#define		DWN_PRESSED		( SW_UP | SW_ESC | SW_PUMP | SW_ENTER )
#define		ESC_PRESSED		(SW_DWN | SW_UP  | SW_PUMP | SW_ENTER )
#define		UP_PRESSED		(SW_DWN | SW_ESC | SW_PUMP | SW_ENTER )
#define		PUMP_PRESSED	(SW_DWN | SW_UP  | SW_ESC  | SW_ENTER )
#define		NONE_PRESSED	(SW_DWN | SW_UP  | SW_ESC  | SW_PUMP | SW_ENTER)



//////////////////////////////////////////////
////
//// Output Ports:
////
//////////////////////////////////////////////


// MAKE SURE THESE AGREE WITH  {XXX_PWM defines below)
#define CONDITIONER_CTRL    PE4	 // TImer OC3A for PWM control	   
#define INITIATOR_CTRL		PE3	 // TImer OC3B for PWM control	

#define MODERATOR_CTRL      PB5  //  Timmer OC1A for PWM control  
#define NOZZLE_CTRL		    PB6	 //  Timmer OC1B for PWM control  
#define SAMPLE_CTRL		    PB7	 //  Timmer OC1C for PWM control


#define TEMP_CTRL_PORT1 PORTE
#define TEMP_CTRL_DDR1  DDRE
#define TEMP_CTRL_PORT2 PORTB
#define TEMP_CTRL_DDR2  DDRB


#define TEMP_CTRL_DDR1_SETTING  ( _BV(CONDITIONER_CTRL) | _BV(INITIATOR_CTRL) )
#define TEMP_CTRL_DDR2_SETTING  ( _BV(MODERATOR_CTRL) | _BV(NOZZLE_CTRL) | _BV(SAMPLE_CTRL) )


// MAKE SURE THESE AGREE WITH  {XXX_CTRL defines above)
#define CONDITIONER_PWM  OCR3B     // PE4
#define INITIATOR_PWM    OCR3A     // PE3     
#define MODERATOR_PWM    OCR1A     // PB5  
#define NOZZLE_PWM       OCR1B     // PB6
#define SAMPLE_PWM       OCR1C     // PB7


#define H20_CTRL			PC0   // water injection pump
#define PUMP_CTRL			PC4   // air pump

#define H20_CTRL_PORT	PORTC
#define H20_CTRL_DDR    DDRC
#define H20_INJECT_ON  	0xFF
#define H20_INJECT_OFF  0
#define PUMP_CTRL_PORT 	PORTC
#define PUMP_CTRL_DDR 	DDRC
#define PUMP_CTRL_ON   	0xFF
#define PUMP_CTRL_OFF  	0



#endif
