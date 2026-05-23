#include <NES/CPU/MOS6502.h>
#include <iostream>

namespace NES::CPU
{

    MOS6502::MOS6502()
    {
        cycles = 0;

        status.setFlag(StatusRegister::FlagID::I, true);
        status.setFlag(StatusRegister::FlagID::U, true);
    }

    void MOS6502::clock()
    {
        cycles++;
    }

    void MOS6502::reset()
    {
        cycles = 0;
    }

    uint64_t MOS6502::getCycles()
    {
        return cycles;
    }

    void MOS6502::mock()
    {
        std::cout << (int)status.getByte() << "\n";

        status.setFlag(StatusRegister::FlagID::D, 1);
        status.setFlag(StatusRegister::FlagID::D, 1);
        std::cout << (int)status.getByte() << "\n";

        status.setFlag(StatusRegister::FlagID::D, 0);
        status.setFlag(StatusRegister::FlagID::D, 0);
        std::cout << (int)status.getByte() << "\n";

        std::cout << "-----SUMAS:-----\n";

        ALUResult result = alu.add(200, 156, 0);
        std::cout << "200 + 156 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.add(2, 4, 0);
        std::cout << "2 + 4 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.add(155, 101, 0);
        std::cout << "155 + 101 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.add(5, 118, 0);
        std::cout << "5 + 118 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.add(5, 124, 0);
        std::cout << "5 + 124 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.add(127, 1, 0);
        std::cout << "127 + 1 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        std::cout << "-----RESTAS:-----\n";

        result = alu.substract(124, 124, 1);
        std::cout << "124 - 124 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(127, 15, 1);
        std::cout << "127 - 15 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(189, 10, 1);
        std::cout << "-67 - 10 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(17, 20, 1);
        std::cout << "17 - 20 = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(253, 252, 1);
        std::cout << "-3 - (-4) = " << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(0x80, 1, 1);
        std::cout << "-128 - 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(0, 1, 1);
        std::cout << "0 - 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.substract(127, 255, 1);
        std::cout << "127 - (-1) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.add(0, 1, 0);
        status.applyMask(result.statusBits, result.statusChangedBits);

        std::cout << "-----BITS:-----\n";

        result = alu.andOp(65, 31);
        std::cout << "65 & 31 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.andOp(128, 127);
        std::cout << "65 & 31 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.andOp(200, 129);
        std::cout << "200 & 129 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.orOp(10, 130);
        std::cout << "10 | 130 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.orOp(0, 0);
        std::cout << "0 | 0 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.andOp(127, 66);
        std::cout << "127 | 66 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.xorOp(10, 7);
        std::cout << "10 ^ 7 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.xorOp(123, 123);
        std::cout << "123 ^ 123 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.shiftLeft(128);
        std::cout << "128 << 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.shiftLeft(78);
        std::cout << "78 << 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.shiftLeft(175);
        std::cout << "175 << 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.shiftRight(1);
        std::cout << "1 >> 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.shiftRight(10);
        std::cout << "10 >> 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.shiftRight(75);
        std::cout << "75 >> 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";


        result = alu.rotateLeft(128, 0);
        std::cout << "128 ROL(0) 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.rotateLeft(128, 1);
        std::cout << "128 ROL(1) 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.rotateLeft(78, 0);
        std::cout << "78 ROL(0) 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.rotateLeft(78, 1);
        std::cout << "78 ROL(1) 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.rotateLeft(175, 0);
        std::cout << "175 ROL(0) 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.rotateLeft(175, 1);
        std::cout << "175 ROL(1) 1 =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";
        
        //Notese que el carry estará activo por la instrucción anterior para los INC

        result = alu.increment(255);
        std::cout << "INC(255) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.increment(0);
        std::cout << "INC(0) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.increment(127);
        std::cout << "INC(127) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.increment(79);
        std::cout << "INC(79) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";



        result = alu.decrement(0);
        std::cout << "DEC(0) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.decrement(1);
        std::cout << "DEC(1) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.decrement(128);
        std::cout << "DEC(128) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.decrement(80);
        std::cout << "DEC(79) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        //-----CMP-----

        result = alu.compare(80,80);
        std::cout << "CMP(80, 80) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.compare(125,127);
        std::cout << "CMP(125, 127) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";

        result = alu.compare(255,0);
        std::cout << "CMP(255, 0) =" << (int)result.result << "\n";
        status.applyMask(result.statusBits, result.statusChangedBits);
        std::cout << (int)status.getByte() << "\n";
    }
}