// Minimal STM32 I2C "v2" (F7/L4/U3 style) controller model.
//
// Renode 1.16's I2C.STM32F7_I2C ignores the byte that Zephyr's stm32 I2C v2 driver preloads
// into TXDR before START (errata workaround) and keeps asserting TXIS, so the driver's ISR
// runs past its buffer. This model honours the preloaded byte.
//
// Only controller mode, 7-bit addressing, interrupt-driven (no DMA) transfers are modelled.
// Targets are attached from the platform description (e.g. `sensor: Sensors.X @ i2c2 0x6a`).

using System;
using System.Collections.Generic;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Peripherals.Helpers;

namespace Antmicro.Renode.Peripherals.I2C
{
    public class STM32U3_I2C : IDoubleWordPeripheral, IKnownSize, IPeripheralContainer<II2CPeripheral, NumberRegistrationPoint<int>>
    {
        private enum Phase { Idle, Write, Read }

        // Register offsets
        private const long Cr1 = 0x00, Cr2 = 0x04, Isr = 0x18, Icr = 0x1C, Rxdr = 0x24, Txdr = 0x28;

        // Registers without behaviour (OAR1/2, TIMINGR, TIMEOUTR, PECR, ...) just read back what was written.
        private const long ShadowSize = 0x40;

        private const uint Cr1Pe = 1u << 0, Cr1Txie = 1u << 1, Cr1Rxie = 1u << 2,
                           Cr1Nackie = 1u << 4, Cr1Stopie = 1u << 5, Cr1Tcie = 1u << 6;
        private const uint Cr2RdWrn = 1u << 10, Cr2Start = 1u << 13, Cr2Stop = 1u << 14, Cr2Nack = 1u << 15,
                           Cr2Reload = 1u << 24, Cr2Autoend = 1u << 25;
        private const uint IsrTxe = 1u << 0, IsrTxis = 1u << 1, IsrRxne = 1u << 2, IsrNackf = 1u << 4,
                           IsrStopf = 1u << 5, IsrTc = 1u << 6, IsrTcr = 1u << 7, IsrBusy = 1u << 15;
        private const uint IcrNackcf = 1u << 4, IcrStopcf = 1u << 5;

        private readonly object sync = new object();
        private readonly SimpleContainerHelper<II2CPeripheral> container;
        private readonly List<byte> writeBuffer = new List<byte>();
        private readonly uint[] shadow = new uint[ShadowSize / 4];

        private uint cr1, cr2;
        // ISR flags
        private bool txe = true, txis, rxne, nackf, stopf, tc, tcr, busy;

        private Phase phase = Phase.Idle;
        private II2CPeripheral target;
        private int bytesLeft;
        private bool txdrFull;
        private byte txdr, rxdr;
        private byte[] readBuffer = Array.Empty<byte>();
        private int readPosition;

        public STM32U3_I2C(IMachine machine)
        {
            container = new SimpleContainerHelper<II2CPeripheral>(machine, this);
            EventInterrupt = new GPIO();
            ErrorInterrupt = new GPIO(); // never raised, only exposed so platform descriptions can connect it
        }

        public GPIO EventInterrupt { get; private set; }

        public GPIO ErrorInterrupt { get; private set; }

        public long Size { get { return 0x400; } }

        public IEnumerable<IRegistered<II2CPeripheral, NumberRegistrationPoint<int>>> Children
        {
            get { return container.Children; }
        }

        public IEnumerable<NumberRegistrationPoint<int>> GetRegistrationPoints(II2CPeripheral peripheral)
        {
            return container.GetRegistrationPoints(peripheral);
        }

        public void Register(II2CPeripheral peripheral, NumberRegistrationPoint<int> registrationPoint)
        {
            container.Register(peripheral, registrationPoint);
        }

        public void Unregister(II2CPeripheral peripheral)
        {
            container.Unregister(peripheral);
        }

        public void Reset()
        {
            lock(sync)
            {
                cr1 = 0;
                cr2 = 0;
                Array.Clear(shadow, 0, shadow.Length);
                ResetTransferState();
                UpdateInterrupts();
            }
        }

        public uint ReadDoubleWord(long offset)
        {
            lock(sync)
            {
                switch(offset)
                {
                    case Cr1:
                        return cr1;
                    case Cr2:
                        return cr2;
                    case Isr:
                        return ReadIsr();
                    case Txdr:
                        return txdr;
                    case Rxdr:
                        var value = rxdr;
                        rxne = false;
                        Advance();
                        UpdateInterrupts();
                        return value;
                    default:
                        return offset < ShadowSize ? shadow[offset / 4] : 0;
                }
            }
        }

        public void WriteDoubleWord(long offset, uint value)
        {
            lock(sync)
            {
                switch(offset)
                {
                    case Cr1:
                        WriteCr1(value);
                        break;
                    case Cr2:
                        WriteCr2(value);
                        break;
                    case Isr:
                        if((value & IsrTxe) != 0) // software flush of TXDR
                        {
                            txdrFull = false;
                            txe = true;
                        }
                        break;
                    case Icr:
                        if((value & IcrNackcf) != 0) { nackf = false; }
                        if((value & IcrStopcf) != 0) { stopf = false; }
                        break;
                    case Txdr:
                        WriteTxdr((byte)value);
                        break;
                    case Rxdr:
                        break; // read-only
                    default:
                        if(offset < ShadowSize)
                        {
                            shadow[offset / 4] = value;
                        }
                        break;
                }
                UpdateInterrupts();
            }
        }

        private uint ReadIsr()
        {
            uint isr = 0;
            if(txe) { isr |= IsrTxe; }
            if(txis) { isr |= IsrTxis; }
            if(rxne) { isr |= IsrRxne; }
            if(nackf) { isr |= IsrNackf; }
            if(stopf) { isr |= IsrStopf; }
            if(tc) { isr |= IsrTc; }
            if(tcr) { isr |= IsrTcr; }
            if(busy) { isr |= IsrBusy; }
            return isr;
        }

        private void WriteCr1(uint value)
        {
            var wasEnabled = (cr1 & Cr1Pe) != 0;
            cr1 = value;

            if(wasEnabled && (cr1 & Cr1Pe) == 0)
            {
                ResetTransferState(); // clearing PE resets the peripheral state machine
            }
        }

        private void WriteCr2(uint value)
        {
            // START/STOP/NACK are write-only strobes, not stored
            cr2 = value & ~(Cr2Start | Cr2Stop | Cr2Nack);

            if((cr1 & Cr1Pe) == 0)
            {
                return;
            }

            if((value & Cr2Start) != 0)
            {
                StartTransfer();
            }
            else if((value & Cr2Stop) != 0)
            {
                Stop();
            }
            else if(tcr && phase != Phase.Idle)
            {
                // New NBYTES written while TCR is pending: continue the reloaded transfer
                tcr = false;
                bytesLeft = Nbytes;
                Advance();
            }
        }

        private void WriteTxdr(byte value)
        {
            txdr = value;
            txdrFull = true;
            txe = false;
            txis = false;

            if(phase == Phase.Write)
            {
                Advance();
            }
        }

        private int Nbytes
        {
            get { return (int)((cr2 >> 16) & 0xFF); }
        }

        private void StartTransfer()
        {
            if(phase != Phase.Idle && target != null)
            {
                target.FinishTransmission(); // repeated start ends the previous phase
            }

            ClearTransferFlags();
            writeBuffer.Clear();

            var address = (int)((cr2 >> 1) & 0x7F);

            if(!container.TryGetByAddress(address, out target))
            {
                this.Log(LogLevel.Warning, "No I2C target at 0x{0:X2}, signalling NACK", address);
                nackf = true;
                stopf = true;
                busy = false;
                phase = Phase.Idle;
                return;
            }

            busy = true;
            bytesLeft = Nbytes;

            if((cr2 & Cr2RdWrn) != 0)
            {
                phase = Phase.Read;
                readBuffer = bytesLeft > 0 ? target.Read(bytesLeft) : Array.Empty<byte>();
                readPosition = 0;
            }
            else
            {
                phase = Phase.Write;
            }

            Advance();
        }

        // Moves the current transfer forward until it has to wait for software (TXIS/RXNE) or is complete.
        private void Advance()
        {
            switch(phase)
            {
                case Phase.Write:
                    AdvanceWrite();
                    break;
                case Phase.Read:
                    AdvanceRead();
                    break;
            }
        }

        private void AdvanceWrite()
        {
            // A byte already in TXDR (e.g. preloaded by the driver before START) is shifted out first
            if(txdrFull && bytesLeft > 0)
            {
                writeBuffer.Add(txdr);
                txdrFull = false;
                txe = true;
                bytesLeft--;
            }

            if(bytesLeft > 0)
            {
                txis = true; // ask software for the next byte
            }
            else
            {
                FinishChunk();
            }
        }

        private void AdvanceRead()
        {
            if(rxne)
            {
                return; // previous byte not yet read by software
            }

            if(bytesLeft > 0)
            {
                rxdr = readBuffer[readPosition++];
                rxne = true;
                bytesLeft--;
            }
            else
            {
                FinishChunk();
            }
        }

        // NBYTES bytes have been transferred: either wait for a reload (TCR) or complete (TC / AUTOEND stop)
        private void FinishChunk()
        {
            if(phase == Phase.Write && writeBuffer.Count > 0)
            {
                target.Write(writeBuffer.ToArray());
                writeBuffer.Clear();
            }

            if((cr2 & Cr2Reload) != 0)
            {
                tcr = true;
                return;
            }

            tc = true;
            if((cr2 & Cr2Autoend) != 0)
            {
                Stop();
            }
        }

        private void Stop()
        {
            if(target != null)
            {
                target.FinishTransmission();
            }

            phase = Phase.Idle;
            busy = false;
            ClearTransferFlags();
            stopf = true;
        }

        private void ClearTransferFlags()
        {
            tc = false;
            tcr = false;
            txis = false;
            rxne = false;
        }

        private void ResetTransferState()
        {
            phase = Phase.Idle;
            txe = true;
            nackf = false;
            stopf = false;
            busy = false;
            txdrFull = false;
            bytesLeft = 0;
            writeBuffer.Clear();
            ClearTransferFlags();
        }

        private void UpdateInterrupts()
        {
            var active = ((cr1 & Cr1Txie) != 0 && txis)
                      || ((cr1 & Cr1Rxie) != 0 && rxne)
                      || ((cr1 & Cr1Tcie) != 0 && (tc || tcr))
                      || ((cr1 & Cr1Stopie) != 0 && stopf)
                      || ((cr1 & Cr1Nackie) != 0 && nackf);
            EventInterrupt.Set(active);
        }
    }
}
