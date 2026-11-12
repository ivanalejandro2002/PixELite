#pragma once
#include <cstdint>
#include <vector>

#include <NES/PPU/Mirroring.h>
#include <NES/SaveStateStream.h>

namespace NES
{
    class Mapper
    {
        protected:
            uint8_t prgBanks = 0;
            uint8_t chrBanks = 0;
        public:
            Mapper(uint8_t prgBanks, uint8_t chrBanks)
                : prgBanks(prgBanks), chrBanks(chrBanks) {}

            virtual ~Mapper() = default;

            virtual void reset() {}

            virtual bool cpuMapRead(uint16_t addr, uint32_t& mappedAddr) = 0;
            virtual bool cpuMapWrite(uint16_t addr, uint32_t& mappedAddr, uint8_t data) = 0;

            virtual bool ppuMapRead(uint16_t addr, uint32_t& mappedAddr) = 0;
            virtual bool ppuMapWrite(uint16_t addr, uint32_t& mappedAddr) = 0;

            virtual void notifyPpuaddress(uint16_t addr) {}

            virtual bool hasDynamicMirroring() const { return false; }
            virtual Mirroring getMapperMirroring() const { return Mirroring::Vertical; }

            virtual bool irqState() const { return false; }
            virtual void irqClear() {}

            virtual void saveState(std::ostream& out) const { (void)out; }
            virtual void loadState(std::istream& in) { (void)in; }
    };
}