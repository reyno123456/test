#include "interrupt.h"
#include "debuglog.h"

#define MAX_IRQ_VECTROS		(99)

static Irq_handler handlers[MAX_IRQ_VECTROS] = {
	0
};

int reg_IrqHandle(IRQ_type vct, Irq_handler hdl)
{
    if(vct < MAX_IRQ_VECTROS)
    {
        handlers[vct] = hdl;
        return 0;
    }

    return 1;
}

int rmv_IrqHandle(IRQ_type vct)
{
    if(vct < MAX_IRQ_VECTROS)
    {
        handlers[vct] = 0;
        return 0;
    }
    return 1;
}

static inline run_irq_hdl(IRQ_type vct)
{
    if(handlers[vct] != 0)
    {
        (handlers[vct])();
    }
}

__attribute__((weak)) void Inc_sysTicks(void)
{
}

__attribute__((weak)) void osSystickHandler(void)
{
}

__attribute__((weak)) void SYSTICK_IRQHandler(void)
{
    Inc_sysTicks();
    osSystickHandler();
}

void IRQHandler_16(void)  { run_irq_hdl(16);  }
void IRQHandler_17(void)  { run_irq_hdl(17);  }
void IRQHandler_18(void)  { run_irq_hdl(18);  }
void IRQHandler_19(void)  { run_irq_hdl(19);  }
void IRQHandler_20(void)  { run_irq_hdl(20);  }
void IRQHandler_21(void)  { run_irq_hdl(21);  }
void IRQHandler_22(void)  { run_irq_hdl(22);  }
void IRQHandler_23(void)  { run_irq_hdl(23);  }
void IRQHandler_24(void)  { run_irq_hdl(24);  }
void IRQHandler_25(void)  { run_irq_hdl(25);  }
void IRQHandler_26(void)  { run_irq_hdl(26);  }
void IRQHandler_27(void)  { run_irq_hdl(27);  }
void IRQHandler_28(void)  { run_irq_hdl(28);  }
void IRQHandler_29(void)  { run_irq_hdl(29);  }
void IRQHandler_30(void)  { run_irq_hdl(30);  }
void IRQHandler_31(void)  { run_irq_hdl(31);  }
void IRQHandler_32(void)  { run_irq_hdl(32);  }
void IRQHandler_33(void)  { run_irq_hdl(33);  }
void IRQHandler_34(void)  { run_irq_hdl(34);  }
void IRQHandler_35(void)  { run_irq_hdl(35);  }
void IRQHandler_36(void)  { run_irq_hdl(36);  }
void IRQHandler_37(void)  { run_irq_hdl(37);  }
void IRQHandler_38(void)  { run_irq_hdl(38);  }
void IRQHandler_39(void)  { run_irq_hdl(39);  }
void IRQHandler_40(void)  { run_irq_hdl(40);  }
void IRQHandler_41(void)  { run_irq_hdl(41);  }
void IRQHandler_42(void)  { run_irq_hdl(42);  }
void IRQHandler_43(void)  { run_irq_hdl(43);  }
void IRQHandler_44(void)  { run_irq_hdl(44);  }
void IRQHandler_45(void)  { run_irq_hdl(45);  }
void IRQHandler_46(void)  { run_irq_hdl(46);  }
void IRQHandler_47(void)  { run_irq_hdl(47);  }
void IRQHandler_48(void)  { run_irq_hdl(48);  }
void IRQHandler_49(void)  { run_irq_hdl(49);  }
void IRQHandler_50(void)  { run_irq_hdl(50);  }
void IRQHandler_51(void)  { run_irq_hdl(51);  }
void IRQHandler_52(void)  { run_irq_hdl(52);  }
void IRQHandler_53(void)  { run_irq_hdl(53);  }
void IRQHandler_54(void)  { run_irq_hdl(54);  }
void IRQHandler_55(void)  { run_irq_hdl(55);  }
void IRQHandler_56(void)  { run_irq_hdl(56);  }
void IRQHandler_57(void)  { run_irq_hdl(57);  }
void IRQHandler_58(void)  { run_irq_hdl(58);  }
void IRQHandler_59(void)  { run_irq_hdl(59);  }
void IRQHandler_60(void)  { run_irq_hdl(60);  }
void IRQHandler_61(void)  { run_irq_hdl(61);  }
void IRQHandler_62(void)  { run_irq_hdl(62);  }
void IRQHandler_63(void)  { run_irq_hdl(63);  }
void IRQHandler_64(void)  { run_irq_hdl(64);  }
void IRQHandler_65(void)  { run_irq_hdl(65);  }
void IRQHandler_66(void)  { run_irq_hdl(66);  }
void IRQHandler_67(void)  { run_irq_hdl(67);  }
void IRQHandler_68(void)  { run_irq_hdl(68);  }
void IRQHandler_69(void)  { run_irq_hdl(69);  }
void IRQHandler_70(void)  { run_irq_hdl(70);  }
void IRQHandler_71(void)  { run_irq_hdl(71);  }
void IRQHandler_72(void)  { run_irq_hdl(72);  }
void IRQHandler_73(void)  { run_irq_hdl(73);  }
void IRQHandler_74(void)  { run_irq_hdl(74);  }
void IRQHandler_75(void)  { run_irq_hdl(75);  }
void IRQHandler_76(void)  { run_irq_hdl(76);  }
void IRQHandler_77(void)  { run_irq_hdl(77);  }
void IRQHandler_78(void)  { run_irq_hdl(78);  }
void IRQHandler_79(void)  { run_irq_hdl(79);  }
void IRQHandler_80(void)  { run_irq_hdl(80);  }
void IRQHandler_81(void)  { run_irq_hdl(81);  }
void IRQHandler_82(void)  { run_irq_hdl(82);  }
void IRQHandler_83(void)  { run_irq_hdl(83);  }
void IRQHandler_84(void)  { run_irq_hdl(84);  }
void IRQHandler_85(void)  { run_irq_hdl(85);  }
void IRQHandler_86(void)  { run_irq_hdl(86);  }
void IRQHandler_87(void)  { run_irq_hdl(87);  }
void IRQHandler_88(void)  { run_irq_hdl(88);  }
void IRQHandler_89(void)  { run_irq_hdl(89);  }
void IRQHandler_90(void)  { run_irq_hdl(90);  }
void IRQHandler_91(void)  { run_irq_hdl(91);  }
void IRQHandler_92(void)  { run_irq_hdl(92);  }
void IRQHandler_93(void)  { run_irq_hdl(93);  }
void IRQHandler_94(void)  { run_irq_hdl(94);  }
void IRQHandler_95(void)  { run_irq_hdl(95);  }
void IRQHandler_96(void)  { run_irq_hdl(96);  }
void IRQHandler_97(void)  { run_irq_hdl(97);  }
void IRQHandler_98(void)  { run_irq_hdl(98);  }

