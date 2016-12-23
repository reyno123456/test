#include "debuglog.h"
#include "interrupt.h"
#include "test_sd.h"

void InitSDIRQ()
{
    /* register the irq handler */
    reg_IrqHandle(SD_INTR_VECTOR_NUM, sd_IRQHandler, NULL);
    INTR_NVIC_EnableIRQ(SD_INTR_VECTOR_NUM);
    INTR_NVIC_SetIRQPriority(SD_INTR_VECTOR_NUM, 1);

}

void TestWR()
{
	InitSDIRQ();

	int res;
	if ((res = sd_init()) != 0)
	{
		return;
	}

	// int i = 0;
	// uint32_t sect = 0;
	// while(i < 10)
	// {
	// 	sd_write(sect, 0x20000000, 1);
	// 	delay_ms(10);
	// 	i++;
	// 	sect += 1;
	// }

 //    sd_write(0x00000000, 0x44000000, 1); /* byte units*/

	// sd_read(0x440F0100, 0x00000000, 4);   /*block units*/
	
	// dlog_info("write finish\n");

	// sd_write(0x00000000, 0x44000000, 1); /* block units*/

	// sd_write(0x00000008, 0x44000000, 4); /* byte units*/

	// sd_read(0x440f0100, 0x00000008,  4);   /*block units*/

	// sd_write(0x0000000a, 0x44000000, 4); /* byte units*/

	// sd_write(0x0000000d, 0x44000000, 4); /* byte units*/


	// sd_write(0x00000010, 0x44000000, 4); /* byte units*/

	// sd_write(0x00000014, 0x44000000, 4); /* byte units*/

	// sd_write(0x00000018, 0x44000000, 4); /* byte units*/

	// sd_write(0x0000001c, 0x44000000, 300);
	//sd_deinit();
}

void TestFatFs()
{
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[100];                                   /* File read buffer */
	/*##-1- Link the micro SD disk I/O driver ##################################*/
	if (FATFS_LinkDriver(&SD_Driver, SDPath) == 0)
	{
		dlog_info("Link success!\n");
		/*##-2- Register the file system object to the FatFs module ##############*/
		if ((res = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
		{
			/* FatFs Initialization Error */
			dlog_info("f_mount = %d\n", res);
			dlog_info("f_mount error!\n");
		}
		else
		{
			dlog_info("f_mount success!\n");
			// res = f_mkfs((TCHAR const*)SDPath, 0, 0);
			// dlog_info("f_mkfs = %d\n", res);
			/*##-4- Create and Open a new text file object with write access #####*/
			if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
			{
				/* 'STM32.TXT' file Open for write Error */
				dlog_info("f_open error!\n");
			}

			else
			{
				dlog_info("f_open success!\n");
				/*##-5- Write data to the text file ################################*/
				res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

				if ((byteswritten == 0) || (res != FR_OK))
				{
					/* 'STM32.TXT' file Write or EOF Error */
					dlog_info("f_write error!\n");
				}
				else
				{
					dlog_info("f_write success!\n");
					/*##-6- Close the open text file #################################*/
					f_close(&MyFile);

					/*##-7- Open the text file object with read access ###############*/
					if (f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
					{
						/* 'STM32.TXT' file Open for read Error */
						dlog_info("f_open error!\n");
					}
					else
					{
						/*##-8- Read data from the text file ###########################*/
						res = f_read(&MyFile, rtext, sizeof(rtext), (UINT*)&bytesread);

						if ((bytesread == 0) || (res != FR_OK))
						{
							/* 'STM32.TXT' file Read or EOF Error */
							dlog_info("f_read error!\n");
						}
						else
						{
							dlog_info("f_read success!\n");
							/*##-9- Close the open text file #############################*/
							f_close(&MyFile);

							/*##-10- Compare read data with the expected data ############*/
							if ((bytesread != byteswritten))
							{
								/* Read data is different from the expected data */
								dlog_info("f_close error!\n");
							}
							else
							{
								/* Success of the demo: no error occurrence */
								dlog_info("f_close success!\n");
							}
						}
					}
				}
			}
		
		}
	}

	/*##-11- Unlink the micro SD disk I/O driver ###############################*/
	res = FATFS_UnLinkDriver(SDPath);
}

void command_initSdcard()
{
    InitSDIRQ();
	int res;
	if ((res = sd_init()) != 0)
	{
		return;
	}
}
