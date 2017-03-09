#ifndef BB_SNR_SERVICE_H__
#define BB_SNR_SERVICE_H__


typedef enum
{
    QAMUP,
    QAMDOWN,
    QAMKEEP
}QAMUPDONW;

uint8_t is_snr_ok(uint16_t iMCS);

void grd_get_snr(void);

void arlink_snr_daq(void);

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

uint16_t get_snr_qam_threshold(void);

uint16_t get_snr_average(void);

uint16_t grd_get_it_snr();

int grd_check_piecewiseSnrPass(uint8_t u8_flag_start, uint16_t u16_thld);

int grd_check_piecewiseSnrPass(uint8_t u8_flag_start, uint16_t u16_thld);

void grd_set_txmsg_mcs_change(uint8_t index );

void grd_judge_qam_mode(void);

#endif /* __CONFIG_H__ */

/************************ (C) COPYRIGHT Artosyn *****END OF FILE****/
