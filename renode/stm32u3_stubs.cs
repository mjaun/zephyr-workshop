// Minimal STM32U3 RCC and PWR stubs.
//
// Registers read back what was written, and the "ready" flags that Zephyr's clock init polls
// are reported as set so it doesn't hang.

using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Peripherals.Bus;

namespace Antmicro.Renode.Peripherals.Miscellaneous
{
    public abstract class STM32U3_RegisterStub : IDoubleWordPeripheral, IKnownSize
    {
        private readonly Dictionary<long, uint> regs = new Dictionary<long, uint>();

        public long Size { get { return 0x400; } }

        public void Reset()
        {
            regs.Clear();
        }

        public uint ReadDoubleWord(long offset)
        {
            uint value;
            regs.TryGetValue(offset, out value);
            return OnRead(offset, value);
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            regs[offset] = value;
        }

        protected abstract uint OnRead(long offset, uint value);
    }

    public class STM32U3_RCC : STM32U3_RegisterStub
    {
        private const long Cr = 0x000, Cfgr1 = 0x01C, Bdcr = 0x110;

        // Oscillator/PLL ready bits polled by Zephyr's clock init
        private const uint CrReady = (1u << 2) | (1u << 4) | (1u << 9) | (1u << 10) | (1u << 13) | (1u << 15) | (1u << 17);
        private const uint Cfgr1Sw = 0x3, Cfgr1Sws = 0xC;
        private const uint BdcrLseOn = 1u << 0, BdcrLseRdy = 1u << 1, BdcrLseSysEn = 1u << 7, BdcrLseSysRdy = 1u << 11;

        protected override uint OnRead(long offset, uint value)
        {
            switch(offset)
            {
                case Cr:
                    // report all oscillators/PLLs ready
                    return value | CrReady;
                case Cfgr1:
                    // SWS[3:2] mirrors SW[1:0]
                    return (value & ~Cfgr1Sws) | ((value & Cfgr1Sw) << 2);
                case Bdcr:
                    // LSERDY/LSESYSRDY follow LSEON/LSESYSEN
                    value &= ~(BdcrLseRdy | BdcrLseSysRdy);
                    if((value & BdcrLseOn) != 0) value |= BdcrLseRdy;
                    if((value & BdcrLseSysEn) != 0) value |= BdcrLseSysRdy;
                    return value;
                default:
                    return value;
            }
        }
    }

    public class STM32U3_PWR : STM32U3_RegisterStub
    {
        private const long Vosr = 0x00C;
        private const uint VosrVosRdy = 1u << 15, VosrBoostRdy = 1u << 24;

        protected override uint OnRead(long offset, uint value)
        {
            // VOSR: voltage scaling / boost ready
            return offset == Vosr ? value | VosrVosRdy | VosrBoostRdy : value;
        }
    }
}
