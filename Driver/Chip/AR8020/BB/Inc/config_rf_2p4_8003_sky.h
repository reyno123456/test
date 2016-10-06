#ifndef __CONFIG_RF_2P4_8003_SKY_H
#define __CONFIG_RF_2P4_8003_SKY_H

/*
        D[31:24]=01 -> Write AR8001(Baseband Register)
        D[31:24]=02 -> Read  AR8001(Baseband Register)
        D[31:24]=03 -> Write AD9363(RF Chip)
        D[31:24]=04 -> Read AD9363(RF Chip)
        D[31:24]=05 -> Write AR8003(RF Chip)
        D[31:24]=06 -> Read AR8003(RF Chip)
        D[31:24]=11 -> Wait  N ms

        D[23:16]=0E -> for AR8001 and AR8003 meas for writting,don't care ,just consider it as a data for spi write
        D[23:16]=0F -> for AR8001 and AR8003 meas for reading, don't care ,just consider it as a data for spi read
        D[23:16]=80 to 83 -> for AD9363 meas for writting ,don't care ,just consider it as a data for spi write
        D[23:16]=00 to 03 -> for AD9363 meas for reading, don't care ,just consider it as a data for spi read
        D[15:8] -> for AR8001 and AR8003 ,it is a  entirely adress but for AD9363,it is the Lower 16 bits address
        D[7:0] -> 8 bits data value

*/
const uint32_t Config_RF_Data[] = {        //  AR8003
      0x010E0101,
      0x05800074,
      0x0580027F,
      0x05800400,
      0x058006C0,
      0x058008C0,
      0x05800a02,
      0x05800c00,
      0x05800e00,
      0x05801034,
      0x05801200,
      0x05801400,
      0x05801600,
      0x05801834,
      0x05801a00,
      0x05801c00,
      0x05801e00,
      0x05802000,
      0x05802280,
      0x05802400,
      0x05802692,
      0x05802800,
      0x05802a50,
      0x05802c00,
      0x05802e00,
      0x05803000,
      0x05803200,
      0x05803400,
      0x05803600,
      0x05803800,
      0x05803a7D,
      0x05803c00,
      0x05803e00,
      0x05804000,
      0x05804240,
      0x05804440,
      0x05804648,
      0x05804800,
      0x05804a54,
      0x05804c00,
      0x05804e00,
      0x0580502A,
      0x05805200,
      0x05805418,
      0x05805600,
      0x0580581F,
      0x05805a00,
      0x05805c18,
      0x05805e07,
      0x0580601F,
      0x05806200,
      0x05806428,
      0x0580664C,
      0x05806800,
      0x05806a70,
      0x05806c80,
      0x05806e00,
      0x05807037,
      0x05807220,
      0x05807404,
      0x05807600,
      0x05807814,
      0x05807a8E,
      0x05807c40,
      0x05807e00,
      0x058042C0,
      0x05804240,
      0x010E0102
};

const uint32_t Config_RF_RC_Tune_Data[]=   //AR8003
  {
      0x010E0101,
      0x05802A51,
      0x11000001,
      0x05802A50,
      0x11000001,
      0x010E0102
  };
const uint32_t Config_RF_NO_Current_Data[]=   //AR8003
  {
      0x010E0101,
      0x05806AB2,
      0x010E0102
  };
#endif
