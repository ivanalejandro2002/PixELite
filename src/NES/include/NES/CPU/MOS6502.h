#pragma once

#include <NES/CPU/StatusRegister.h>
#include <NES/CPU/ALU.h>
#include <NES/Bus/Bus.h>
#include <NES/CPU/Instruction/Instruction.h>
#include <NES/CPU/Instruction/InstructionTable.h>
#include <NES/CPU/Instruction/InstructionFactory.h>
#include <optional>

namespace NES::CPU
{
    class MOS6502
    {
        public:
            MOS6502();

            void clock();
            void reset();

            uint64_t getCycles();

            void connectBus(Bus *b);

            void execute(MicroOperation operation);

            void mock();
        private:
            uint8_t A;
            uint8_t X;
            uint8_t Y;
            uint16_t PC;
            uint8_t S;
            StatusRegister status;
            
            ALU alu;

            NES::Bus* bus = nullptr;

            uint8_t opcode;

            uint8_t data;
            uint8_t addressLow;
            uint8_t addressHigh;

            uint16_t address;

            InstructionTable instructionTable;
            InstructionFactory instructionFactory;
            
            std::optional<Instruction> currentInstruction;

            uint64_t cycles;


            // Micro operaciones

            void fetchOpcode();
            void fetchOperand();
            
            void fetchAddressLow();
            void fetchAddressHigh();

            void fetchZeroPageAddress();

            void fetchIndirectLow();
            void fetchIndirectHigh();

            void addXToAddress();
            void addYToAddress();

            void addXToZeroPageAddress();
            void addYToZeroPageAddress();

            void readOperand();
            void writeOperand();

            void dummyRead();

            void execute();
            void evaluateBranch();

            // Instrucciones

            void executeInstruction();

            // Instrucciones Legales

            void executeLDA();
            void executeLDX();
            void executeLDY();
            
            void executeSTA();
            void executeSTX();
            void executeSTY();
            
            void executeTAX();
            void executeTAY();
            void executeTXA();
            void executeTYA();

            void executeADC();
            void executeSBC();
            void executeINC();
            void executeDEC();
            void executeINX();
            void executeDEX();
            void executeINY();
            void executeDEY();

            void executeASL();
            void executeLSR();
            void executeROL();
            void executeROR();
            
            void executeAND();
            void executeORA();
            void executeEOR();
            void executeBIT();

            void executeCMP();
            void executeCPX();
            void executeCPY();
            
            void executeBCC();
            void executeBCS();
            void executeBEQ();
            void executeBNE();
            void executeBPL();
            void executeBMI();
            void executeBVC();
            void executeBVS();

            void executeJMP();
            void executeJSR();
            void executeRTS();
            void executeBRK();
            void executeRTI();

            void executePHA();
            void executePLA();
            void executePHP();
            void executePLP();
            void executeTXS();
            void executeTSX();

            void executeCLC();
            void executeSEC();
            void executeCLI();
            void executeSEI();
            void executeCLD();
            void executeSED();
            void executeCLV();

            void executeNOP();

            // Instrucciones Ilegales

            void executeSLO();
            void executeRLA();
            void executeSRE();
            void executeRRA();
            void executeSAX();
            void executeLAX();
            void executeDCP();
            void executeISC();

            void executeANC();
            void executeALR();
            void executeARR();
            void executeXAA();
            void executeAXS();

            void executeAHX();
            void executeSHX();
            void executeSHY();

            void executeTAS();
            void executeLAS();

            void executeKIL();
            void executeUndefined();
    };
}
