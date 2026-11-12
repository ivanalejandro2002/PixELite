#pragma once
#include <NES/Device.h>
#include <NES/SaveStateStream.h>

#include <array>

namespace NES
{
    class RAM: public Device
    {
        public:
            bool handlesRead(uint16_t addr)const override;
            bool handlesWrite(uint16_t addr)const override;
            
            uint8_t read(uint16_t addr)override;
            void write(uint16_t addr, uint8_t value)override;

            void saveState(std::ostream& out) const {
                SaveStateStream::writeArray(out, memory);
            }

            void loadState(std::istream& in) {
                SaveStateStream::readArray(in, memory);
            }
        
        private:
            std::array<uint8_t, 2048> memory;
    };
}