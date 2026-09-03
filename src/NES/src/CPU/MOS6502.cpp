#include <NES/CPU/MOS6502.h>
#include <NES/Bus/Bus.h>
#include <iostream>

namespace NES::CPU
{

    MOS6502::MOS6502()
    {
        cycles = 0;

        status.setFlag(StatusRegister::FlagID::I, true);
        status.setFlag(StatusRegister::FlagID::U, true);
    }

    void MOS6502::connectBus(Bus *b)
    {
        bus = b;
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

    void MOS6502::execute(MicroOperation operation)
    {
        switch(operation)
        {
            case MicroOperation::FetchOpcode:
                fetchOpcode();
                break;

            case MicroOperation::FetchOperand:
                fetchOperand();
                break;

            case MicroOperation::FetchAddressLow:
                fetchAddressLow();
                break;

            case MicroOperation::FetchAddressHigh:
                fetchAddressHigh();
                break;

            case MicroOperation::FetchZeroPageAddress:
                fetchZeroPageAddress();
                break;
            
            case MicroOperation::FetchIndirectLow:
                fetchIndirectLow();
                break;

            case MicroOperation::FetchIndirectHigh:
                fetchIndirectHigh();
                break;

            case MicroOperation::ReadOperand:
                readOperand();
                break;

            case MicroOperation::WriteOperand:
                writeOperand();
                break;

            case MicroOperation::DummyRead:
                dummyRead();
                break;

            case MicroOperation::AddXToAddress:
                addXToAddress();
                break;

            case MicroOperation::AddYToAddress:
                addYToAddress();
                break;

            case MicroOperation::AddXToZeroPageAddress:
                addXToZeroPageAddress();
                break;
            
            case MicroOperation::AddYToZeroPageAddress:
                addYToZeroPageAddress();
                break;
            
            case MicroOperation::Execute:
                execute();
                break;

            case MicroOperation::EvaluateBranch:
                evaluateBranch();
                break;

            default:
                break;
        }
    }

    void MOS6502::executeInstruction()
    {
        if(!currentInstruction.has_value())
        {
            return;
        }
        switch(currentInstruction->getOperation())
        {
            case InstructionOperation::LDA:
                executeLDA();
                break;
            
            case InstructionOperation::LDX:
                executeLDX();
                break;
            
            case InstructionOperation::LDY:
                executeLDY();
                break;
            


            case InstructionOperation::STA:
                executeSTA();
                break;
            
            case InstructionOperation::STX:
                executeSTX();
                break;
            
            case InstructionOperation::STY:
                executeSTY();
                break;

            
            
            case InstructionOperation::TAX:
                executeTAX();
                break;

            case InstructionOperation::TAY:
                executeTAY();
                break;
            
            case InstructionOperation::TXA:
                executeTXA();
                break;

            case InstructionOperation::TYA:
                executeTYA();
                break;

            

            case InstructionOperation::ADC:
                executeADC();
                break;

            case InstructionOperation::SBC:
                executeSBC();
                break;

            case InstructionOperation::INC:
                executeINC();
                break;

            case InstructionOperation::DEC:
                executeDEC();
                break;

            case InstructionOperation::INX:
                executeINX();
                break;

            case InstructionOperation::DEX:
                executeDEX();
                break;

            case InstructionOperation::INY:
                executeINY();
                break;

            case InstructionOperation::DEY:
                executeDEY();
                break;

            


            case InstructionOperation::ASL:
                executeASL();
                break;

            case InstructionOperation::LSR:
                executeLSR();
                break;

            case InstructionOperation::ROL:
                executeROL();
                break;

            case InstructionOperation::ROR:
                executeROR();
                break;



            case InstructionOperation::AND:
                executeAND();
                break;

            case InstructionOperation::ORA:
                executeORA();
                break;

            case InstructionOperation::EOR:
                executeEOR();
                break;

            case InstructionOperation::BIT:
                executeBIT();
                break;

            


            case InstructionOperation::CMP:
                executeCMP();
                break;

            case InstructionOperation::CPX:
                executeCPX();
                break;

            case InstructionOperation::CPY:
                executeCPY();
                break;
            


            case InstructionOperation::BCC:
                executeBCC();
                break;

            case InstructionOperation::BCS:
                executeBCS();
                break;

            case InstructionOperation::BEQ:
                executeBEQ();
                break;

            case InstructionOperation::BNE:
                executeBNE();
                break;

            case InstructionOperation::BPL:
                executeBPL();
                break;

            case InstructionOperation::BMI:
                executeBMI();
                break;

            case InstructionOperation::BVC:
                executeBVC();
                break;

            case InstructionOperation::BVS:
                executeBVS();
                break;



            case InstructionOperation::JMP:
                executeJMP();
                break;

            case InstructionOperation::JSR:
                executeJSR();
                break;

            case InstructionOperation::RTS:
                executeRTS();
                break;

            case InstructionOperation::BRK:
                executeBRK();
                break;

            case InstructionOperation::RTI:
                executeRTI();
                break;



            case InstructionOperation::PHA:
                executePHA();
                break;

            case InstructionOperation::PLA:
                executePLA();
                break;

            case InstructionOperation::PHP:
                executePHP();
                break;

            case InstructionOperation::PLP:
                executePLP();
                break;

            case InstructionOperation::TXS:
                executeTXS();
                break;

            case InstructionOperation::TSX:
                executeTSX();
                break;



            case InstructionOperation::CLC:
                executeCLC();
                break;

            case InstructionOperation::SEC:
                executeSEC();
                break;

            case InstructionOperation::CLI:
                executeCLI();
                break;

            case InstructionOperation::SEI:
                executeSEI();
                break;

            case InstructionOperation::CLD:
                executeCLD();
                break;

            case InstructionOperation::SED:
                executeSED();
                break;

            case InstructionOperation::CLV:
                executeCLV();
                break;



            case InstructionOperation::NOP:
                executeNOP();
                break;



            case InstructionOperation::SLO:
                executeSLO();
                break;

            case InstructionOperation::RLA:
                executeRLA();
                break;

            case InstructionOperation::SRE:
                executeSRE();
                break;

            case InstructionOperation::RRA:
                executeRRA();
                break;

            case InstructionOperation::SAX:
                executeSAX();
                break;

            case InstructionOperation::LAX:
                executeLAX();
                break;

            case InstructionOperation::DCP:
                executeDCP();
                break;

            case InstructionOperation::ISC:
                executeISC();
                break;



            case InstructionOperation::ANC:
                executeANC();
                break;

            case InstructionOperation::ALR:
                executeALR();
                break;

            case InstructionOperation::ARR:
                executeARR();
                break;

            case InstructionOperation::XAA:
                executeXAA();
                break;

            case InstructionOperation::AXS:
                executeAXS();
                break;



            case InstructionOperation::AHX:
                executeAHX();
                break;

            case InstructionOperation::SHX:
                executeSHX();
                break;

            case InstructionOperation::SHY:
                executeSHY();
                break;



            case InstructionOperation::TAS:
                executeTAS();
                break;

            case InstructionOperation::LAS:
                executeLAS();
                break;



            case InstructionOperation::KIL:
                executeKIL();
                break;

            

            case InstructionOperation::Undefined:
                executeUndefined();
                break;
        }
    }


    // Micro operaciones

    void MOS6502::fetchOpcode()
    {

    }
    void MOS6502::fetchOperand()
    {

    }
    
    void MOS6502::fetchAddressLow()
    {

    }
    void MOS6502::fetchAddressHigh()
    {

    }

    void MOS6502::fetchZeroPageAddress()
    {

    }

    void MOS6502::fetchIndirectLow()
    {

    }
    void MOS6502::fetchIndirectHigh()
    {

    }

    void MOS6502::addXToAddress()
    {

    }
    void MOS6502::addYToAddress()
    {

    }

    void MOS6502::addXToZeroPageAddress()
    {

    }
    void MOS6502::addYToZeroPageAddress()
    {

    }

    void MOS6502::readOperand()
    {

    }
    void MOS6502::writeOperand()
    {

    }

    void MOS6502::dummyRead()
    {

    }

    void MOS6502::execute()
    {

    }
    void MOS6502::evaluateBranch()
    {

    }

    // Instrucciones

    // Instrucciones Legales

    void MOS6502::executeLDA()
    {

    }
    void MOS6502::executeLDX()
    {

    }
    void MOS6502::executeLDY()
    {

    }
    
    void MOS6502::executeSTA()
    {

    }
    void MOS6502::executeSTX()
    {

    }
    void MOS6502::executeSTY()
    {

    }
    
    void MOS6502::executeTAX()
    {

    }
    void MOS6502::executeTAY()
    {

    }
    void MOS6502::executeTXA()
    {

    }
    void MOS6502::executeTYA()
    {

    }

    void MOS6502::executeADC()
    {

    }
    void MOS6502::executeSBC()
    {

    }
    void MOS6502::executeINC()
    {

    }
    void MOS6502::executeDEC()
    {

    }
    void MOS6502::executeINX()
    {

    }
    void MOS6502::executeDEX()
    {

    }
    void MOS6502::executeINY()
    {

    }
    void MOS6502::executeDEY()
    {

    }

    void MOS6502::executeASL()
    {

    }
    void MOS6502::executeLSR()
    {

    }
    void MOS6502::executeROL()
    {

    }
    void MOS6502::executeROR()
    {

    }
    
    void MOS6502::executeAND()
    {

    }
    void MOS6502::executeORA()
    {

    }
    void MOS6502::executeEOR()
    {

    }
    void MOS6502::executeBIT()
    {

    }

    void MOS6502::executeCMP()
    {

    }
    void MOS6502::executeCPX()
    {

    }
    void MOS6502::executeCPY()
    {

    }
    
    void MOS6502::executeBCC()
    {

    }
    void MOS6502::executeBCS()
    {

    }
    void MOS6502::executeBEQ()
    {

    }
    void MOS6502::executeBNE()
    {

    }
    void MOS6502::executeBPL()
    {

    }
    void MOS6502::executeBMI()
    {

    }
    void MOS6502::executeBVC()
    {

    }
    void MOS6502::executeBVS()
    {

    }

    void MOS6502::executeJMP()
    {

    }
    void MOS6502::executeJSR()
    {

    }
    void MOS6502::executeRTS()
    {

    }
    void MOS6502::executeBRK()
    {

    }
    void MOS6502::executeRTI()
    {

    }

    void MOS6502::executePHA()
    {

    }
    void MOS6502::executePLA()
    {

    }
    void MOS6502::executePHP()
    {

    }
    void MOS6502::executePLP()
    {

    }
    void MOS6502::executeTXS()
    {

    }
    void MOS6502::executeTSX()
    {

    }

    void MOS6502::executeCLC()
    {

    }
    void MOS6502::executeSEC()
    {

    }
    void MOS6502::executeCLI()
    {

    }
    void MOS6502::executeSEI()
    {

    }
    void MOS6502::executeCLD()
    {

    }
    void MOS6502::executeSED()
    {

    }
    void MOS6502::executeCLV()
    {

    }

    void MOS6502::executeNOP()
    {

    }

    // Instrucciones Ilegales

    void MOS6502::executeSLO()
    {

    }
    void MOS6502::executeRLA()
    {

    }
    void MOS6502::executeSRE()
    {

    }
    void MOS6502::executeRRA()
    {

    }
    void MOS6502::executeSAX()
    {

    }
    void MOS6502::executeLAX()
    {

    }
    void MOS6502::executeDCP()
    {

    }
    void MOS6502::executeISC()
    {

    }

    void MOS6502::executeANC()
    {

    }
    void MOS6502::executeALR()
    {

    }
    void MOS6502::executeARR()
    {

    }
    void MOS6502::executeXAA()
    {

    }
    void MOS6502::executeAXS()
    {

    }

    void MOS6502::executeAHX()
    {

    }
    void MOS6502::executeSHX()
    {

    }
    void MOS6502::executeSHY()
    {

    }

    void MOS6502::executeTAS()
    {

    }
    void MOS6502::executeLAS()
    {

    }

    void MOS6502::executeKIL()
    {

    }
    void MOS6502::executeUndefined()
    {

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
        std::cout << "-----INCREMENTOS:-----\n";

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
        std::cout << "-----COMPARACIONES:-----\n";

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


        //-----Lectura y Escritura-----
        std::cout << "-----LECTURA Y ESCRITURA RAM:-----\n";

        uint8_t s = bus->read(0x1FFF);
        std::cout << "VALOR ORIGINAL EN: 0X1FFF: " << (int)s << "\n";

        bus->write(0x17FF, 99);

        s = bus->read(0x0FFF);

        std::cout << "VALOR FINAL MAPEADO EN: 0X0FFF: " << (int)s << "\n";

        //-----Creación de Instrucciones------
        std::cout << "-----CREACIÓN DE INSTRUCCIONES:-----\n";

        InstructionDefinition definition = instructionTable.get(0x69);
        Instruction mockInstruction = instructionFactory.create(definition);

        mockInstruction.printInstruction();

        definition = instructionTable.get(0x71);
        mockInstruction = instructionFactory.create(definition);

        mockInstruction.printInstruction();

        definition = instructionTable.get(0x61);
        mockInstruction = instructionFactory.create(definition);

        mockInstruction.printInstruction();

        definition = instructionTable.get(0xA5);
        mockInstruction = instructionFactory.create(definition);

        mockInstruction.printInstruction();
    }
}
