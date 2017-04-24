#ifndef IT_66021_H_
#define IT_66021_H_


#define IT_66021_REG_BLOCK_SEL          (0x0F)  

/*                        
#define REG_RX_P0_SYS_STATUS            (0x0A)
#define B_P0_PWR5V_DET                  (0x01)
*/

#define IT_66021_V_LOCKED_SET_INTERRUPT_POS                    (0x1)
#define IT_66021_V_LOCKED_SET_INTERRUPT_MASK                   (0x1)
#define IT_66021_V_LOCKED_CLEAR_INTERRUPT_POS                  (0x1)
#define IT_66021_V_LOCKED_CLEAR_INTERRUPT_MASK                 (0x1)

#define IT_66021_DE_REGEN_LCK_SET_INTERRUPT_POS                (0x0)
#define IT_66021_DE_REGEN_LCK_SET_INTERRUPT_MASK               (0x1)
#define IT_66021_DE_REGEN_LCK_CLEAR_INTERRUPT_POS              (0x0)
#define IT_66021_DE_REGEN_LCK_CLEAR_INTERRUPT_MASK             (0x1)


uint8_t IT_66021_IrqHandler0(void);
uint8_t IT_66021_IrqHandler1(void);
void IT_66021_Initial(uint8_t index);
void IT_66021_DumpOutEdidData(uint8_t index);
void IT_66021_DumpOutDefaultSettings(uint8_t index);
void IT_66021_GetVideoFormat(uint8_t index, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr);
void IT_66021_GetAudioSampleRate(uint8_t index, uint32_t* sampleRate);
void IT_66021_GetVideo(uint8_t index, uint16_t* widthPtr, uint16_t* hightPtr, uint8_t* framteratePtr);
void IT_66021_WriteByte(uint8_t slv_addr, uint8_t sub_addr, uint8_t val);
uint8_t IT_66021_ReadByte(uint8_t slv_addr, uint8_t sub_addr);
void IT_66021_Set(unsigned char slv_addr, unsigned char sub_addr, unsigned char mask, unsigned char val);
void IT_66021_ChangeBank(uint8_t index, uint8_t bank);


#endif

