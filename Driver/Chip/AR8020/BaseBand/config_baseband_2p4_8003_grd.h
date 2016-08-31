#ifndef __CONFIG_BASEBAND_2P4_8003_GRD_H
#define __CONFIG_BASEBAND_2P4_8003_GRD_H

/*      config_baseband_2p4_8003_grd
        D[31:24]=01 -> Write AR8001(Baseband Register)
        D[31:24]=02 -> Read  AR8001(Baseband Register)
        D[31:24]=03 -> Write AD9363(RF Chip)
        D[31:24]=04 -> Read  AD9363(RF Chip)
        D[31:24]=05 -> Write AR8003(RF Chip)
        D[31:24]=06 -> Read  AR8003(RF Chip)
        D[31:24]=11 -> Wait  N ms

        D[23:16]=0E -> for AR8001 and AR8003 meas for writting,don't care ,just consider it as a data for spi write
        D[23:16]=0F -> for AR8001 and AR8003 meas for reading, don't care ,just consider it as a data for spi read
        D[23:16]=80 to 83 -> for AD9363 meas for writting ,don't care ,just consider it as a data for spi write
        D[23:16]=00 to 03 -> for AD9363 meas for reading, don't care ,just consider it as a data for spi read
        D[15:8] -> for AR8001 and AR8003 ,it is a  entirely adress but for AD9363,it is the Lower 16 bits address
        D[7:0] -> 8 bits data value

*/
const uint32_t Config_Baseband_Data[] = {
      0x010E00B8,
      0x010E0102,
      0x010E0200,
      0x010E03FF,
      0x010E04FF,
      0x010E05FF,
      0x010E0680,
      0x010E07FF,
      0x010E080F,
      0x010E090D,
      0x010E0A00,
      0x010E0B40,
      0x010E0C00,
      0x010E0D40,
      0x010E0E00,
      0x010E0F00,
      0x010E10FC,
      0x010E1187,
      0x010E1200,
      0x010E1310,
      0x010E1400,
      0x010E1502,
      0x010E1600,
      0x010E1700,
      0x010E18A0,
      0x010E1940,
      0x010E1A00,
      0x010E1B00,
      0x010E1C00,
      0x010E1D00,
      0x010E1E00,
      0x010E1FFF,
      0x010E209C,
      0x010E210A,
      0x010E221B,
      0x010E2377,
      0x010E2402,
      0x010E2571,
      0x010E267A,
      0x010E2712,
      0x010E2877,
      0x010E29A1,
      0x010E2A77,
      0x010E2BA1,
      0x010E2C02,
      0x010E2D71,
      0x010E2E0C,
      0x010E2FF1,
      0x010E3000,
      0x010E3100,
      0x010E3200,
      0x010E3300,
      0x010E3400,
      0x010E3500,
      0x010E3600,
      0x010E3700,
      0x010E3800,
      0x010E3900,
      0x010E3A00,
      0x010E3B00,
      0x010E3C00,
      0x010E3D00,
      0x010E3E04,
      0x010E3FE2,
      0x010E4008,
      0x010E4130,
      0x010E428A,
      0x010E4373,
      0x010E4400,
      0x010E4500,
      0x010E4600,
      0x010E4700,
      0x010E4800,
      0x010E4900,
      0x010E4A04,
      0x010E4B18,
      0x010E4C20,
      0x010E4D14,
      0x010E4E06,
      0x010E4F03,
      0x010E5000,
      0x010E5100,
      0x010E5200,
      0x010E5300,
      0x010E5400,
      0x010E5500,
      0x010E5600,
      0x010E5700,
      0x010E5800,
      0x010E5900,
      0x010E5A00,
      0x010E5B00,
      0x010E5C00,
      0x010E5D00,
      0x010E5E00,
      0x010E5F00,
      0x010E6000,
      0x010E6100,
      0x010E6200,
      0x010E6300,
      0x010E6400,
      0x010E6500,
      0x010E6600,
      0x010E6700,
      0x010E6800,
      0x010E6900,
      0x010E6A00,
      0x010E6B00,
      0x010E6C00,
      0x010E6D00,
      0x010E6E00,
      0x010E6F00,
      0x010E7000,
      0x010E7100,
      0x010E7200,
      0x010E7300,
      0x010E7400,
      0x010E7500,
      0x010E7600,
      0x010E7700,
      0x010E7800,
      0x010E7900,
      0x010E7A00,
      0x010E7B00,
      0x010E7C00,
      0x010E7D00,
      0x010E7E00,
      0x010E7F00,
      0x010E8010,//
      0x010E817A,//
      0x010E8200,
      0x010E8300,
      0x010E8400,
      0x010E8500,
      0x010E8600,
      0x010E8700,
      0x010E8800,
      0x010E8900,
      0x010E8A02,
      0x010E8B01,
      0x010E8C00,
      0x010E8D09,
      0x010E8E01,
      0x010E8F00,
      0x010E9000,  // FC (2016.2.7)
      0x010E9100,  // FF (2016.2.7)
      0x010E9200,  // 06
      0x010E9300,  // 00
      0x010E9400,  // 00
      0x010E9500,  // F4
      0x010E9600,  // 1E
      0x010E9700,  // A2
      0x010E9800,  // 0F
      0x010E9900,  //1 // 1D
      0x010E9A00,  // 20
      0x010E9B00,
      0x010E9C00,
      0x010E9D00,
      0x010E9E00,
      0x010E9F00,
      0x010EA004,
      0x010EA100,  // 0F --> reg_tx_cali_on
      0x010EA200,
      0x010EA300,
      0x010EA450,
      0x010EA500,
      0x010EA600,
      0x010EA700,
      0x010EA800,
      0x010EA900,
      0x010EAA00,
      0x010EAB00,
      0x010EAC00,
      0x010EAD00,
      0x010EAE00,
      0x010EAF00,
      0x010EB000,
      0x010EB100,
      0x010EB20F,
      0x010EB300,
      0x010EB400,
      0x010EB50F,
      0x010EB600,
      0x010EB732,
      0x010EB832,
      0x010EB97C,
      0x010EBA00,
      0x010EBB00,
      0x010EBC00,
      0x010EBD00,
      0x010EBE00,
      0x010EBF00,
      0x010EC01C,
      0x010EC122,
      0x010EC22A,
      0x010EC33A,
      0x010EC4AF,
      0x010EC5F1,
      0x010EC60F,
      0x010EC71F,
      0x010EC82F,
      0x010EC93E,
      0x010ECAB5,
      0x010ECBF6,
      0x010ECC00,
      0x010ECD00,
      0x010ECE04,
      0x010ECF00,
      0x010ED000,
      0x010ED100,
      0x010ED200,
      0x010ED300,
      0x010ED400,
      0x010ED500,
      0x010ED600,
      0x010ED700,
      0x010ED800,
      0x010ED900,
      0x010EDA00,
      0x010EDB00,
      0x010EDC00,
      0x010EDD00,
      0x010EDE00,
      0x010EDF00,
      0x010EE000,
      0x010EE100,
      0x010EE200,
      0x010EE300,
      0x010EE400,
      0x010EE500,
      0x010EE600,
      0x010EE700,
      0x010EE800,
      0x010EE900,
      0x010EEA00,
      0x010EEB00,
      0x010EEC00,
      0x010EED00,
      0x010EEE00,
      0x010EEF00,
      0x010EF000,
      0x010EF100,
      0x010EF200,
      0x010EF300,
      0x010EF400,
      0x010EF500,
      0x010EF600,
      0x010EF700,
      0x010EF800,
      0x010EF900,
      0x010EFA00,
      0x010EFB00,
      0x010EFC00,
      0x010EFD00,
      0x010EFE00,
      0x010EFF00,
      0x010E0038,
      0x010E0102,
      0x010E0215,
      0x010E0302,
      0x010E0464,
      0x010E0560,
      0x010E0602,
      0x010E0764,
      0x010E0880,
      0x010E09DF,
      0x010E0A03,
      0x010E0B46,
      0x010E0C43,
      0x010E0D36,
      0x010E0E04,
      0x010E0FFF,
      0x010E1035,
      0x010E11C8,
      0x010E122C,
      0x010E1325,
      0x010E140F,
      0x010E151F,
      0x010E161F,
      0x010E171F,
      0x010E1808,
      0x010E1915,
      0x010E1A13,
      0x010E1B12,
      0x010E1C1C,
      0x010E1DFF,
      0x010E1E0F,
      0x010E1F00,
      0x010E2027,
      0x010E2160,
      0x010E2200,
      0x010E2300,
      0x010E2420,
      0x010E2500,
      0x010E2653,
      0x010E2796,
      0x010E28C9,
      0x010E2974,
      0x010E2A08,
      0x010E2B00,
      0x010E2C00,
      0x010E2D00,
      0x010E2E05,
      0x010E2F71,
      0x010E3019,
      0x010E3100,
      0x010E3260,
      0x010E330A,
      0x010E3402,
      0x010E3590,
      0x010E3652,
      0x010E370A,
      0x010E3830,
      0x010E390A,
      0x010E3A05,
      0x010E3B21,
      0x010E3C45,
      0x010E3D14,
      0x010E3E53,
      0x010E3F31,
      0x010E4028,
      0x010E4191,
      0x010E4245,
      0x010E430A,
      0x010E4430,
      0x010E4552,
      0x010E4602,
      0x010E4700,
      0x010E4820,
      0x010E4901,
      0x010E4A00,
      0x010E4B09,
      0x010E4C01,
      0x010E4D00,
      0x010E4E10,
      0x010E4F10,
      0x010E500F,
      0x010E510A,
      0x010E5254,
      0x010E5337,
      0x010E5477,
      0x010E5577,
      0x010E5672,
      0x010E5723,
      0x010E5844,
      0x010E5945,
      0x010E5A57,
      0x010E5B70,
      0x010E5C87,
      0x010E5D9F,
      0x010E5EBF,
      0x010E5FC9,
      0x010E6000,
      0x010E6150,
      0x010E6231,
      0x010E6319,
      0x010E6488,
      0x010E6535,
      0x010E6605,
      0x010E6780,
      0x010E68D0,
      0x010E6934,
      0x010E6A0B,
      0x010E6BC0,
      0x010E6C00,
      0x010E6D01,
      0x010E6E40,
      0x010E6F01,
      0x010E7001,
      0x010E7100,
      0x010E7200,
      0x010E7300,
      0x010E7408,
      0x010E7598,
      0x010E76FF,
      0x010E77FF,
      0x010E78FF,
      0x010E7942,
      0x010E7A71,
      0x010E7B90,
      0x010E7CFF,
      0x010E7DFF,
      0x010E7EFF,
      0x010E7F19,
      0x010E8000,
      0x010E8120,
      0x010E82C4,
      0x010E8337,
      0x010E8413,
      0x010E8584,
      0x010E8608,
      0x010E8753,
      0x010E8875,
      0x010E8997,
      0x010E8AAA,
      0x010E8B03,
      0x010E8C00,
      0x010E8D00,
      0x010E8E0C,
      0x010E8F00,
      0x010E9048,
      0x010E9199,
      0x010E922A,
      0x010E931A,
      0x010E9401,
      0x010E9508,
      0x010E9600,
      0x010E9700,
      0x010E9800,
      0x010E9900,
      0x010E9A10,
      0x010E9B00,
      0x010E9C00,
      0x010E9DDC,
      0x010E9E18,
      0x010E9F86,
      0x010EA02E,
      0x010EA1AC,
      0x010EA2FF,
      0x010EA370,
      0x010EA46B,
      0x010EA55C,
      0x010EA648,
      0x010EA740,
      0x010EA870,
      0x010EA96B,
      0x010EAA5C,
      0x010EAB48,
      0x010EAC40,
      0x010EAD00,
      0x010EAE00,
      0x010EAF00,
      0x010EB02D,
      0x010EB1DF,
      0x010EB2F2,
      0x010EB37C,
      0x010EB42D,
      0x010EB5DF,
      0x010EB6F2,
      0x010EB77C,
      0x010EB86F,
      0x010EB9F9,
      0x010EBA96,
      0x010EBB7B,
      0x010EBC00,
      0x010EBD00,
      0x010EBE00,
      0x010EBF00,
      0x010EC018,
      0x010EC101,
      0x010EC200,
      0x010EC3B9,
      0x010EC400,
      0x010EC500,
      0x010EC600,
      0x010EC700,
      0x010EC800,
      0x010EC900,
      0x010ECA00,
      0x010ECB00,
      0x010ECC00,
      0x010ECD00,
      0x010ECE00,
      0x010ECF00,
      0x010ED000,
      0x010ED100,
      0x010ED200,
      0x010ED300,
      0x010ED400,
      0x010ED500,
      0x010ED600,
      0x010ED700,
      0x010ED800,
      0x010ED900,
      0x010EDA00,
      0x010EDB00,
      0x010EDC00,
      0x010EDD00,
      0x010EDE00,
      0x010EDF00,
      0x010EE000,
      0x010EE100,
      0x010EE200,
      0x010EE300,
      0x010EE400,
      0x010EE500,
      0x010EE600,
      0x010EE700,
      0x010EE800,
      0x010EE900,
      0x010EEA00,
      0x010EEB00,
      0x010EEC00,
      0x010EED00,
      0x010EEE00,
      0x010EEF00,
      0x010EF000,
      0x010EF100,
      0x010EF200,
      0x010EF300,
      0x010EF400,
      0x010EF500,
      0x010EF600,
      0x010EF700,
      0x010EF800,
      0x010EF900,
      0x010EFA00,
      0x010EFB00,
      0x010EFC00,
      0x010EFD00,
      0x010EFE00,
      0x010EFF00
};
#endif

