/*
 * LCDLib.c
 *
 *  Created on: Mar 2, 2017
 *      Author: juanh
 */

#include "LCDLib.h"
#include "msp.h"
#include "driverlib.h"
#include "AsciiLib.h"

/************************************  Global Variables  *******************************************/

/************************************  Global Variables  *******************************************/

/************************************  Private Functions  *******************************************/

/*
 * Resetting Screen to full size.
 * This function should be called after
 * draw_rectangle() if using set_point()
 */

void LCD_SetFullSize(void)
{
    LCD_WriteReg(HOR_ADDR_START_POS, 0);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, 240-1);  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, 320-1); /* Vertical GRAM End Address */
}
/*
 * _delay x ms
 */
void _delay(unsigned long interval)
{
    int i = 0, j = 0;

        for (j = 0; j < interval; j++) {
            for (i = 47861; i > 0; i--);
        }
}

/*******************************************************************************
 * Function Name  : LCD_initSPI
 * Description    : Configures LCD Control lines
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
static void LCD_initSPI()
{
    /*
     * Configuring the spi
     */
    EUSCI_B3->CTLW0 = EUSCI_B_CTLW0_SWRST;
    EUSCI_B3->CTLW0 = EUSCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH |
                      EUSCI_B_CTLW0_SSEL__SMCLK                 |
                      EUSCI_B_SPI_3PIN                          |
                      EUSCI_B_CTLW0_MST                         |
                      EUSCI_B_CTLW0_MSB                         |
                      EUSCI_B_CTLW0_SYNC;
    EUSCI_B3->BRW = 2;
    EUSCI_B3->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;

    P10->SEL0 |= BIT1|BIT2|BIT3;
    P10->SEL1 &= ~(BIT1|BIT2|BIT3);
    P10->DIR |= BIT4|BIT5;   //P10.4 - LCD CS//P10.5 - TP CS
    P10->OUT |= BIT4|BIT5;
}

/*******************************************************************************
 * Function Name  : LCD_reset
 * Description    : Resets LCD
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : Uses P10.0 for reset
 *******************************************************************************/
static void LCD_reset()
{
    P10DIR |= BIT0;
    P10OUT |= BIT0;  // high
    _delay(100);
    P10OUT &= ~BIT0; // low
    _delay(100);
    P10OUT |= BIT0;  // high
}

/************************************  Private Functions  *******************************************/


/************************************  Public Functions  *******************************************/

/*******************************************************************************
 * Function Name  : LCD_DrawRectangle
 * Description    : Draw a rectangle as the specified color
 * Input          : xStart, xEnd, yStart, yEnd, Color
 * Output         : None
 * Return         : None
 * Attention      : Must draw from left to right, top to bottom!
 *******************************************************************************/
void LCD_DrawRectangle(int16_t xStart, int16_t xEnd, int16_t yStart, int16_t yEnd, uint16_t Color)
{
    int32_t dimension = (xEnd - xStart)*(yEnd-yStart);

    LCD_WriteReg(HOR_ADDR_START_POS, yStart);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, yEnd-1);  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, xStart);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, xEnd-1); /* Vertical GRAM End Address */

    LCD_WriteReg(HORIZONTAL_GRAM_SET, yStart);
    LCD_WriteReg(VERTICAL_GRAM_SET, xStart);

    LCD_WriteIndex(DATA_IN_GRAM);
    SPI_CS_LOW;
    LCD_Write_Data_Start();
    while(dimension--){
        LCD_Write_Data_Only(Color);
    }
    SPI_CS_HIGH;

}

/******************************************************************************
 * Function Name  : PutChar
 * Description    : Lcd screen displays a character
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - ASCI: Displayed character
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor)
{
    uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  // get font data
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            //__delay(50);
            if( (tmp_char >> 7 - j) & 0x01 == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  // Character color
            }
        }
    }
}

/******************************************************************************
* Function Name  : PutIcon
* Description    : Lcd screen displays an icon
* Input          : - Xpos: Horizontal coordinate
*                  - Ypos: Vertical coordinate
*                  - IconLib: Displayed ICON
*                  - IconColor: Icon color
* Output         : None
* Return         : None
* Attention      : None
*******************************************************************************/
/*inline void PutIcon( uint16_t Xpos, uint16_t Ypos, Icon_t IconLib, uint16_t IconColor)
{
    uint32_t buffer[64], tmp_char;
    GetIconCode(buffer,IconLib);  // get Icon data
    for(uint32_t  i=0; i<64; i++ )
    {
        tmp_char = buffer[i];
        for(uint32_t  j=0; j<32; j++ )
        {
            //__delay(50);
            if( (tmp_char >> 31 - j) & 0x01 == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, IconColor );  // Character color
            }
        }
    }
}*/

/******************************************************************************
 * Function Name  : GUI_Text
 * Description    : Displays the string
 * Input          : - Xpos: Horizontal coordinate
 *                  - Ypos: Vertical coordinate
 *                  - str: Displayed string
 *                  - charColor: Character color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str, uint16_t Color)
{
    uint8_t TempChar;

    /* Set area back to span the entire LCD */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    do
    {
        TempChar = *str++;
        PutChar( Xpos, Ypos, TempChar, Color);
        if( Xpos < MAX_SCREEN_X - 8)
        {
            Xpos += 8;
        }
        else if ( Ypos < MAX_SCREEN_X - 16)
        {
            Xpos = 0;
            Ypos += 16;
        }
        else
        {
            Xpos = 0;
            Ypos = 0;
        }
    }
    while ( *str != 0 );
}


/*******************************************************************************
 * Function Name  : LCD_Clear
 * Description    : Fill the screen as the specified color
 * Input          : - Color: Screen Color
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Clear(uint16_t Color)
{
    uint32_t loop = SCREEN_SIZE;
    LCD_WriteIndex(DATA_IN_GRAM);
    SPI_CS_LOW;
    LCD_Write_Data_Start();
    while(loop--){
    LCD_Write_Data_Only(Color);
    }
    SPI_CS_HIGH;
}

/******************************************************************************
 * Function Name  : LCD_SetPoint
 * Description    : Drawn at a specified point coordinates
 * Input          : - Xpos: Row Coordinate
 *                  - Ypos: Line Coordinate
 * Output         : None
 * Return         : None
 * Attention      : 18N Bytes Written
 *******************************************************************************/
void LCD_SetPoint(uint16_t Xpos, uint16_t Ypos, uint16_t color)
{
    LCD_WriteReg(HORIZONTAL_GRAM_SET, Ypos);
    LCD_WriteReg(VERTICAL_GRAM_SET, Xpos);
    LCD_WriteReg(DATA_IN_GRAM, color);
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Only
 * Description    : Data writing to the LCD controller
 * Input          : - data: data to be written
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Only(uint16_t data)
{
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */
}

/*******************************************************************************
 * Function Name  : LCD_WriteData
 * Description    : LCD write register data
 * Input          : - data: register data
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteData(uint16_t data)
{
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
    SPISendRecvByte((data >>   8));                    /* Write D8..D15                */
    SPISendRecvByte((data & 0xFF));                    /* Write D0..D7                 */

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Reads the selected LCD Register.
 * Input          : None
 * Output         : None
 * Return         : LCD Register Value.
 * Attention      : None
 *******************************************************************************/
inline uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
    LCD_WriteIndex(LCD_Reg);
    SPI_CS_LOW;
    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);
    SPISendRecvByte(0);
    SPISendRecvByte(0);
    SPISendRecvByte(0);
    SPISendRecvByte(0);
    SPISendRecvByte(0);
    uint16_t value = SPISendRecvByte(0)<<8;
    value |= SPISendRecvByte(0);
    SPI_CS_HIGH;
    return value;
}

/*******************************************************************************
 * Function Name  : LCD_WriteIndex
 * Description    : LCD write register address
 * Input          : - index: register address
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteIndex(uint16_t index)
{
    SPI_CS_LOW;

    /* SPI write data */
    SPISendRecvByte(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0  */
    SPISendRecvByte(0);
    SPISendRecvByte(index);

    SPI_CS_HIGH;
}

/*******************************************************************************
 * Function Name  : SPISendRecvByte
 * Description    : Send one byte then receive one byte of response
 * Input          : uint8_t: byte
 * Output         : None
 * Return         : Recieved value
 * Attention      : None
 *******************************************************************************/
inline uint8_t SPISendRecvByte (uint8_t byte)
{
    while(!(EUSCI_B3->IFG & EUSCI_B_IFG_TXIFG0));
    EUSCI_B3->TXBUF = byte;
    while(!(EUSCI_B3->IFG & EUSCI_B_IFG_RXIFG));
    return EUSCI_B3->RXBUF;
}

/*******************************************************************************
 * Function Name  : LCD_Write_Data_Start
 * Description    : Start of data writing to the LCD controller
 * Input          : None
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_Write_Data_Start(void)
{
    SPISendRecvByte(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0 */
}

/*******************************************************************************
 * Function Name  : LCD_ReadData
 * Description    : LCD read data
 * Input          : None
 * Output         : None
 * Return         : return data
 * Attention      : Diagram (d) in datasheet
 *******************************************************************************/
inline uint16_t LCD_ReadData()
{
    uint16_t value;
    SPI_CS_LOW;

    SPISendRecvByte(SPI_START | SPI_RD | SPI_DATA);   /* Read: RS = 1, RW = 1   */
    SPISendRecvByte(0);                               /* Dummy read 1           */
    value = (SPISendRecvByte(0) << 8);                /* Read D8..D15           */
    value |= SPISendRecvByte(0);                      /* Read D0..D7            */

    SPI_CS_HIGH;
    return value;
}

/*******************************************************************************
 * Function Name  : LCD_WriteReg
 * Description    : Writes to the selected LCD register.
 * Input          : - LCD_Reg: address of the selected register.
 *                  - LCD_RegValue: value to write to the selected register.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
    LCD_WriteIndex(LCD_Reg);
    LCD_WriteData(LCD_RegValue);
}

/*******************************************************************************
 * Function Name  : LCD_SetCursor
 * Description    : Sets the cursor position.
 * Input          : - Xpos: specifies the X position.
 *                  - Ypos: specifies the Y position.
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
inline void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos )
{
    LCD_DrawRectangle(Xpos,Xpos+1,Ypos,Ypos+8,LCD_WHITE);
    _delay(250);
    LCD_DrawRectangle(Xpos,Xpos+1,Ypos,Ypos+8,LCD_BLACK);
    _delay(250);
}

/*******************************************************************************
 * Function Name  : LCD_Init
 * Description    : Configures LCD Control lines, sets whole screen black
 * Input          : bool usingTP: determines whether or not to enable TP interrupt
 * Output         : None
 * Return         : None
 * Attention      : None
 *******************************************************************************/
void LCD_Init(bool usingTP)
{
    LCD_initSPI();

    if (usingTP)
    {
        //__disable_irq();

        /* Enable Interrupt on P4.0 for the TAP. */
        GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN0);
        GPIO_interruptEdgeSelect(GPIO_PORT_P4, GPIO_PIN0, GPIO_HIGH_TO_LOW_TRANSITION);
        P4->IFG = 0;
        P4->IE |= 1;

        /* Enable Interrupt on Timer32.1 for lcd debounce. */
        Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_256,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
        Timer32_enableInterrupt(TIMER32_0_BASE);

        Interrupt_enableInterrupt(INT_PORT4);
        Interrupt_enableInterrupt(INT_T32_INT1);
       //__enable_irq();
    }

    LCD_reset();

    LCD_WriteReg(0xE5, 0x78F0); /* set SRAM internal timing */
    LCD_WriteReg(DRIVER_OUTPUT_CONTROL, 0x0100); /* set Driver Output Control */
    LCD_WriteReg(DRIVING_WAVE_CONTROL, 0x0700); /* set 1 line inversion */
    LCD_WriteReg(ENTRY_MODE, 0x1038); /* set GRAM write direction and BGR=1 */
    LCD_WriteReg(RESIZING_CONTROL, 0x0000); /* Resize register */
    LCD_WriteReg(DISPLAY_CONTROL_2, 0x0207); /* set the back porch and front porch */
    LCD_WriteReg(DISPLAY_CONTROL_3, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
    LCD_WriteReg(DISPLAY_CONTROL_4, 0x0000); /* FMARK function */
    LCD_WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_1, 0x0000); /* RGB interface setting */
    LCD_WriteReg(FRAME_MARKER_POSITION, 0x0000); /* Frame marker Position */
    LCD_WriteReg(RGB_DISPLAY_INTERFACE_CONTROL_2, 0x0000); /* RGB interface polarity */

    /* Power On sequence */
    LCD_WriteReg(POWER_CONTROL_1, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(POWER_CONTROL_2, 0x0007); /* DC1[2:0], DC0[2:0], VC[2:0] */
    LCD_WriteReg(POWER_CONTROL_3, 0x0000); /* VREG1OUT voltage */
    LCD_WriteReg(POWER_CONTROL_4, 0x0000); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(DISPLAY_CONTROL_1, 0x0001);
    _delay(200);

    /* Dis-charge capacitor power voltage */
    LCD_WriteReg(POWER_CONTROL_1, 0x1090); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
    LCD_WriteReg(POWER_CONTROL_2, 0x0227); /* Set DC1[2:0], DC0[2:0], VC[2:0] */
    _delay(50); /* _delay 50ms */
    LCD_WriteReg(POWER_CONTROL_3, 0x001F);
    _delay(50); /* _delay 50ms */
    LCD_WriteReg(POWER_CONTROL_4, 0x1500); /* VDV[4:0] for VCOM amplitude */
    LCD_WriteReg(POWER_CONTROL_7, 0x0027); /* 04 VCM[5:0] for VCOMH */
    LCD_WriteReg(FRAME_RATE_AND_COLOR_CONTROL, 0x000D); /* Set Frame Rate */
    _delay(50); /* _delay 50ms */
    LCD_WriteReg(GRAM_HORIZONTAL_ADDRESS_SET, 0x0000); /* GRAM horizontal Address */
    LCD_WriteReg(GRAM_VERTICAL_ADDRESS_SET, 0x0000); /* GRAM Vertical Address */

    /* Adjust the Gamma Curve */
    LCD_WriteReg(GAMMA_CONTROL_1,    0x0000);
    LCD_WriteReg(GAMMA_CONTROL_2,    0x0707);
    LCD_WriteReg(GAMMA_CONTROL_3,    0x0307);
    LCD_WriteReg(GAMMA_CONTROL_4,    0x0200);
    LCD_WriteReg(GAMMA_CONTROL_5,    0x0008);
    LCD_WriteReg(GAMMA_CONTROL_6,    0x0004);
    LCD_WriteReg(GAMMA_CONTROL_7,    0x0000);
    LCD_WriteReg(GAMMA_CONTROL_8,    0x0707);
    LCD_WriteReg(GAMMA_CONTROL_9,    0x0002);
    LCD_WriteReg(GAMMA_CONTROL_10,   0x1D04);

    /* Set GRAM area */
    LCD_WriteReg(HOR_ADDR_START_POS, 0x0000);     /* Horizontal GRAM Start Address */
    LCD_WriteReg(HOR_ADDR_END_POS, (MAX_SCREEN_Y - 1));  /* Horizontal GRAM End Address */
    LCD_WriteReg(VERT_ADDR_START_POS, 0x0000);    /* Vertical GRAM Start Address */
    LCD_WriteReg(VERT_ADDR_END_POS, (MAX_SCREEN_X - 1)); /* Vertical GRAM Start Address */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X60, 0x2700); /* Gate Scan Line */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X61, 0x0001); /* NDL,VLE, REV */
    LCD_WriteReg(GATE_SCAN_CONTROL_0X6A, 0x0000); /* set scrolling line */

    /* Partial Display Control */
    LCD_WriteReg(PART_IMAGE_1_DISPLAY_POS, 0x0000);
    LCD_WriteReg(PART_IMG_1_START_END_ADDR_0x81, 0x0000);
    LCD_WriteReg(PART_IMG_1_START_END_ADDR_0x82, 0x0000);
    LCD_WriteReg(PART_IMAGE_2_DISPLAY_POS, 0x0000);
    LCD_WriteReg(PART_IMG_2_START_END_ADDR_0x84, 0x0000);
    LCD_WriteReg(PART_IMG_2_START_END_ADDR_0x85, 0x0000);

    /* Panel Control */
    LCD_WriteReg(PANEL_ITERFACE_CONTROL_1, 0x0010);
    LCD_WriteReg(PANEL_ITERFACE_CONTROL_2, 0x0600);
    LCD_WriteReg(DISPLAY_CONTROL_1, 0x0133); /* 262K color and display ON */
    _delay(50); /* _delay 50 ms */

    //LCD_Clear(LCD_WHITE);
}

/*******************************************************************************
 * Function Name  : TP_ReadXY
 * Description    : Obtain X and Y touch coordinates
 * Input          : None
 * Output         : None
 * Return         : Pointer to "Point" structure
 * Attention      : None
 *******************************************************************************/
Point TP_ReadXY()
{
    Point XY;

    SPI_CS_TP_LOW;
    SPISendRecvByte(CHY);
    XY.y = SPISendRecvByte(0)<<8;
    XY.y |= SPISendRecvByte(0);
    XY.y >>= 4;     //>>=3
    XY.y -= 140;

    SPISendRecvByte(CHX);
    XY.x = SPISendRecvByte(0)<<8;
    XY.x |= SPISendRecvByte(0);
    XY.x >>= 4;     //>>=3
    XY.x -= 190;
    SPI_CS_TP_HIGH;

    XY.x = (float)(XY.x)*(.000565)*MAX_SCREEN_X;
    XY.y = (float)(XY.y)*(.000560)*MAX_SCREEN_Y;
    return XY;
}

/************************************  Public Functions  *******************************************/

/************************  Interrupt Functions for Touch Screen and Dbounce  ***********************/

