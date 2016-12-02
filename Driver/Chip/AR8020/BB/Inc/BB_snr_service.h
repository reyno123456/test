#ifndef BB_SNR_SERVICE_H__
#define BB_SNR_SERVICE_H__

#ifdef __cplusplus
 extern "C" {
#endif


uint8_t is_snr_ok(uint16_t iMCS);

void grd_add_snr_daq(void); //daq --> Data Acquisition

void code_rate_adj(void);

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

uint16_t get_snr_qam_threshold(void);

uint16_t get_snr_average(uint8_t row_index);


#ifdef __cplusplus
}
#endif

#endif /* __CONFIG_H__ */

/************************ (C) COPYRIGHT Artosyn *****END OF FILE****/
