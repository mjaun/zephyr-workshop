// Minimal LSM6DSO16IS I2C stub.
//
// Register file that reads back writes, reports WHO_AM_I and data-ready, and
// lets tests inject the Z acceleration via SetAccelZ().

using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.I2C;

namespace Antmicro.Renode.Peripherals.Sensors
{
    public class LSM6DSO16IS_Stub : II2CPeripheral, IPeripheral
    {
        private const byte WhoAmI = 0x0F;
        private const byte Ctrl3C = 0x12;
        private const byte Ctrl3CSwReset = 0x01;
        private const byte Ctrl3CBoot = 0x80;
        private const byte StatusReg = 0x1E;
        private const byte OutZLowA = 0x2C;
        private const byte WhoAmIValue = 0x22;
        private const double LsbMg = 0.122; // +-4 g full scale
        private const double Gravity = 9.80665;

        private readonly byte[] registers = new byte[256];
        private int pointer;
        private bool pointerSet;

        public LSM6DSO16IS_Stub()
        {
            Reset();
        }

        public void SetAccelZ(double mps2)
        {
            var raw = (short)Math.Round(mps2 / Gravity * 1000.0 / LsbMg);
            registers[OutZLowA] = (byte)(raw & 0xFF);
            registers[OutZLowA + 1] = (byte)((raw >> 8) & 0xFF);
        }

        public void Write(byte[] data)
        {
            this.Log(LogLevel.Noisy, "Write: " + BitConverter.ToString(data));

            if(data.Length == 0)
            {
                return;
            }

            var offset = 0;

            if(!pointerSet)
            {
                pointer = data[0];
                pointerSet = true;
                offset = 1;
            }

            for(var i = offset; i < data.Length; i++)
            {
                var value = data[i];

                if((pointer & 0xFF) == Ctrl3C)
                {
                    value &= unchecked((byte)~(Ctrl3CSwReset | Ctrl3CBoot)); // self-clearing bits
                }

                registers[pointer & 0xFF] = value;
                pointer++;
            }
        }

        public byte[] Read(int count = 1)
        {
            this.Log(LogLevel.Noisy, "Read count " + count);

            var result = new byte[count];

            for(var i = 0; i < count; i++)
            {
                result[i] = ReadRegister(pointer & 0xFF);
                pointer++;
            }

            return result;
        }

        public void FinishTransmission()
        {
            this.Log(LogLevel.Noisy, "FinishTransmission");
            pointerSet = false;
        }

        public void Reset()
        {
            Array.Clear(registers, 0, registers.Length);
            pointer = 0;
            pointerSet = false;
        }

        private byte ReadRegister(int address)
        {
            switch(address)
            {
                case WhoAmI:
                    return WhoAmIValue;
                case StatusReg:
                    return 0x01; // XLDA
                default:
                    return registers[address];
            }
        }
    }
}
