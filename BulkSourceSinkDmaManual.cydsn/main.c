/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>

#define     PutChar(arg)            UART_UartPutChar(arg)

#ifdef      UART_DEBUG
#define     Debug_PutChar(arg)      PutChar(arg)
#else       // !define(UART_DEBUG)
#define     Debug_PutChar(arg)    
#endif      // define(UART_DEBUG)

#define     IN_EP               (0x02u)
#define     OUT_EP              (0x01u)
#define     BUFFER_SIZE         (64u)

#define     ST_RECEIVING        (1u)
#define     ST_PROCESSING       (2u)
#define     ST_PREPARING        (3u)
#define     ST_SENDING          (4u)

uint8       state_out;          // State code for BULK-OUT
uint8       state_in;           // State code for BULK-IN

uint8       buffer_in[BUFFER_SIZE] = "@@ABCDEFGIHJKLMNOPQRSTUVWXYZ";
uint8       buffer_out[BUFFER_SIZE];
uint16      length_out;

int main(void) {
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    UART_Start();

    /* Start USBFS Operation with 3V operation */
    USB_Start(0u, USB_3V_OPERATION);


    for (;;) {
        /* Place your application code here. */
        /* Wait for Device to enumerate */
        while(USB_GetConfiguration() == 0);
        PutChar('C');

        // Drop CHANGE flag
        USB_IsConfigurationChanged();

        /* BULK-OUT: Enable OUT endpoint for receive data from Host */
        // Prepare for next packet
        USB_EnableOutEP(OUT_EP);
        state_out = ST_RECEIVING;

        /* BULK-IN: Specify the first state */
        state_in = ST_PREPARING;

        for (;;) {
            // Check configuration changes
            if (USB_IsConfigurationChanged()) {
                PutChar('c');
                break;
            }

            // BULK-OUT state machine
            switch (state_out) {
                case ST_RECEIVING:
                    // Waiting for buffer FULL
                    if (USB_GetEPState(OUT_EP) == USB_OUT_BUFFER_FULL) {
                        Debug_PutChar('r');
                        /* Read received bytes count */
                        length_out = USB_GetEPCount(OUT_EP);
                        /* Initiate DMA to unload the OUT buffer */
                        USB_ReadOutEP(OUT_EP, &buffer_out[0], length_out);
                        // Prepare for next packet
                        USB_EnableOutEP(OUT_EP);
                        state_out = ST_PROCESSING;
                    }
                    break;
                case ST_PROCESSING:
                    // Process the received buffer
                    Debug_PutChar('p');
                    state_out = ST_RECEIVING;
                    break;
                default:
                    break;
            }
            
            // BULK-IN state machine
            switch (state_in) {
                case ST_PREPARING:
                    // Preparing a packet on the IN buffer
                    Debug_PutChar('P');
                    buffer_in[0]++;
                    state_in = ST_SENDING;
                    break;
                case ST_SENDING:
                    // Send a USB packet
                    if (USB_GetEPState(IN_EP) == USB_IN_BUFFER_EMPTY) {
                        Debug_PutChar('S');
                        // Reload the IN buffer to EP
                        USB_LoadInEP(IN_EP, &buffer_in[0], BUFFER_SIZE);
                        state_in = ST_PREPARING;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

/* [] END OF FILE */
