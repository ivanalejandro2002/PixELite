#pragma once
#include <cstdint>
#include <NES/Device.h>

namespace NES
{
    class Controller : public Device {
        public:
            enum Button{
                A = 1 << 0,
                B = 1 << 1,
                SELECT = 1 << 2,
                START = 1 << 3,
                UP = 1 << 4,
                DOWN = 1 << 5,
                LEFT = 1 << 6,
                RIGHT = 1 << 7
            };

            Controller();

            bool handlesRead(uint16_t addr) const override;
            bool handlesWrite(uint16_t addr) const override;
            uint8_t read(uint16_t addr) override;
            void write(uint16_t addr, uint8_t value) override;

            void setButtonState(Button button, bool pressed);

        private:
            uint8_t buttonStates = 0;
            uint8_t shiftRegister = 0;
            bool strobeState = false;
    };
}