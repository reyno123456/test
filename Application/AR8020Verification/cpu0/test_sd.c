#include "debuglog.h"
#include "interrupt.h"
#include "test_sd.h"

uint8_t pcm_buffer[2048];
FIL outFile, inFile;

void TestWR()
{
    uint32_t info;
	ENUM_HAL_SD_CTRL cmd = HAL_SD_GET_SECTOR_COUNT;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_SECTOR_SIZE;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_CSD_VERSION;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_MANUID;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_TRAN_SPEED;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}

	cmd = HAL_SD_GET_CARD_STATUS;
	if (HAL_SD_Ioctl(cmd, &info) != HAL_OK)
	{
		dlog_info("ioctl failed\n");
	}


	int i = 0;
	uint32_t sect = 0;
	while(i < 10)
	{
		HAL_SD_Write(sect, 0x81000000+i*0x1400, 10);
		HAL_SD_Read(0x81100000, sect, 2);
		dlog_info("%d",i);
		i++;
		sect += 1;
	}

	// HAL_SD_Deinit();
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

void TestFatFs1()
{
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread;                     /* File write/read counts */

    int read, write;


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
			if (f_open(&outFile, "a.mp3", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
			{
				/* 'STM32.TXT' file Open for write Error */
				dlog_info("f_open out error!\n");
			}

			if (f_open(&inFile, "a.wav", FA_READ) != FR_OK)
			{
				/* 'STM32.TXT' file Open for write Error */
				dlog_info("f_open in error!\n");
			}

			
			dlog_info("f_open success!\n");
			/*##-5- Write data to the text file ################################*/
			
			do
			{
				res = f_read(&inFile, pcm_buffer, 4096, &read);

				if ((read == 0) || (res != FR_OK))
				{
					dlog_info("f_read error!\n");
				}
				else
				{
					// dlog_info("read %d!\n",read);
					res = f_write(&outFile, pcm_buffer, read, (void *)&write);
				}
			} while(read > 0);

			dlog_info("complete!\n");
			f_close(&inFile);
			f_close(&outFile);
		
		}
	}

	/*##-11- Unlink the micro SD disk I/O driver ###############################*/
	res = FATFS_UnLinkDriver(SDPath);
}

void TestFatFs2()
{
	FRESULT res;                                          /* FatFs function common result code */
	uint32_t byteswritten, bytesread = 1;                     /* File write/read counts */
	uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
	uint8_t rtext[2048];                                   /* File read buffer */
	FIL fileIn;
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
			// if (f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
			// {
			// 	/* 'STM32.TXT' file Open for write Error */
			// 	dlog_info("f_open error!\n");
			// }
			if (f_open(&fileIn, "a.wav", FA_READ) != FR_OK)
			{
				/* 'STM32.TXT' file Open for write Error */
				dlog_info("f_open error!\n");
			}
			else
			{
				dlog_info("f_open success!\n");
				/*##-5- Write data to the text file ################################*/
				while(bytesread)
				{
				res = f_read(&fileIn, rtext, 1024, (void *)&bytesread);
				dlog_info("read %d \n", bytesread);
				}


			}
		
		}
	}

	/*##-11- Unlink the micro SD disk I/O driver ###############################*/
	res = FATFS_UnLinkDriver(SDPath);
}


void command_initSdcard()
{
    HAL_SD_InitIRQ();
	int res;
	if ((res = HAL_SD_Init(SDR50)) != 0)
	{
		return;
	}

}


void command_SdcardFatFs()
{	
    TestWR();
    // TestFatFs();
/*    TestFatFs1(); */
//	TestFatFs2();
}
