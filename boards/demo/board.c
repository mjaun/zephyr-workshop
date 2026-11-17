#include <zephyr/init.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_crs.h>

#if defined(CONFIG_USB_DEVICE_STACK_NEXT) || defined(CONFIG_USB_DEVICE_STACK)

/*
 * Trim the HSI48 oscillator (USB clock) to the USB start-of-frame packets sent
 * by the host every 1 ms, to meet the USB full-speed clock accuracy.
 * The CRS reset values already select USB SOF as sync source with the
 * default reload and error limit values for 48 MHz.
 */
static int board_usb_crs_init(void)
{
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CRS);
	LL_CRS_SetSyncSignalSource(LL_CRS_SYNC_SOURCE_USB);
	LL_CRS_EnableAutoTrimming();
	LL_CRS_EnableFreqErrorCounter();

	return 0;
}

SYS_INIT(board_usb_crs_init, PRE_KERNEL_2, 0);

#endif
