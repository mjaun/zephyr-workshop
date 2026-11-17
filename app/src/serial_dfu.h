#ifndef SERIAL_DFU_H_
#define SERIAL_DFU_H_

/**
 * Confirms the running image and starts receiving firmware images on the
 * dfu_uart serial port (USB CDC ACM). See serial_dfu.c for the protocol.
 */
void serial_dfu_start(void);

#endif /* SERIAL_DFU_H_ */
