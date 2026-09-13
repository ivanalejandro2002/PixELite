#include <NES/CPU/MOS6502.h>
#include <NES/Bus/Bus.h>
#include <iostream>
#include <cassert>
#include <iomanip>

namespace NES::CPU
{

    MOS6502::MOS6502()
    {
        cycles = 0;
        PC = 0x0000;
        S = 0x00;
        isJammed = 0;
        pendingIUpdates = 0;
        iUpdate = 0;

        status.setFlag(StatusRegister::FlagID::I, true);
        status.setFlag(StatusRegister::FlagID::U, true);

        currentInstruction = instructionFactory.create({
            InstructionOperation::TurnOn,
            AddressingMode::Implied,
            AccessType::None,
            7});
    }

    void MOS6502::connectBus(Bus *b)
    {
        bus = b;
    }

    void MOS6502::clock()
    {
        if(isJammed)return;
        cycles++;

        if(!currentInstruction.has_value() || currentInstruction->finished())
        {
            fetchOpcode();
            currentInstruction = instructionFactory.create(instructionTable.get(opcode));

            // std::cout << "<" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << PC << ">";
            // std::cout << InstructionOperationMapper::ToString(currentInstruction->getOperation()) << "("
            //             << AddressingModeMapper::ToString(currentInstruction->getAddressingMode()) << "), ";

            // currentInstruction->printInstruction();
            return;
        }
        currentInstruction->tick(*this);
    }

    void MOS6502::reset()
    {
        cycles = 0;
        isJammed = 0;
        pendingIUpdates = 0;
        iUpdate = 0;

        currentInstruction = instructionFactory.create({
            InstructionOperation::Reset,
            AddressingMode::Implied,
            AccessType::None,
            7});
    }

    uint64_t MOS6502::getCycles()
    {
        return cycles;
    }

    void MOS6502::finishInstruction()
    {
        if(currentInstruction.has_value())
            currentInstruction->endTask();
    }

    void MOS6502::incrementPC()
    {
        PC = static_cast<uint16_t>(PC + 1);
    }

    void MOS6502::decrementS()
    {
        S = static_cast<uint8_t>(S - 1);
    }

    void MOS6502::incrementS()
    {
        S = static_cast<uint8_t>(S + 1);
    }

    void MOS6502::execute(MicroOperation operation)
    {
        switch(operation)
        {
            case MicroOperation::FetchOpcode:
                fetchOpcode();
                break;

            case MicroOperation::FetchOpcodeAndDiscard:
                fetchOpcodeAndDiscard();
                break;

            case MicroOperation::FetchOpcodeDiscardAndExec:
                fetchOpcodeDiscardAndExec();
                break;

            case MicroOperation::DummyRead:
                dummyRead();
                break;
            
            case MicroOperation::DummyWrite:
                dummyWrite();
                break;

            case MicroOperation::FetchOperand:
                fetchOperand();
                break;
            
            case MicroOperation::ReadOperand:
                readOperand();
                break;

            case MicroOperation::WriteOperand:
                writeOperand();
                break;
            
            case MicroOperation::ReadOperandAndExecute:
                readOperandAndExecute();
                break;
            
            case MicroOperation::ReadOperandVariable:
                readOperandVariable();
                break;

            case MicroOperation::FetchZeroPageAddress:
                fetchZeroPageAddress();
                break;

            case MicroOperation::FetchAddressLow:
                fetchAddressLow();
                break;

            case MicroOperation::FetchAddressHigh:
                fetchAddressHigh();
                break;

            case MicroOperation::FetchAddressHighExecute:
                fetchAddresshighExecute();
                break;

            case MicroOperation::FetchAddressHighAddX:
                fetchAddressHighAddX();
                break;

            case MicroOperation::FetchAddressHighAddY:
                fetchAddressHighAddY();
                break;
            
            case MicroOperation::FetchIndirectLow:
                fetchIndirectLow();
                break;

            case MicroOperation::FetchIndirectHigh:
                fetchIndirectHigh();
                break;

            case MicroOperation::FetchIndirectHighAddY:
                fetchIndirectHighAddY();
                break;
            
            case MicroOperation::FetchIndirectHighExecute:
                fetchIndirectHighExecute();
                break;

            case MicroOperation::AddXToZeroPageAddress:
                addXToZeroPageAddress();
                break;
            
            case MicroOperation::AddYToZeroPageAddress:
                addYToZeroPageAddress();
                break;
            
            case MicroOperation::FetchBranch:
                fetchBranch();
                break;

            case MicroOperation::EvaluateBranch:
                evaluateBranch();
                break;

            case MicroOperation::DummyBranchPageCrossRead:
                dummyBranchPageCrossRead();
                break;

            case MicroOperation::PushStatus:
                pushStatus();
                break;

            case MicroOperation::PushPCHigh:
                pushPCHigh();
                break;

            case MicroOperation::PushPCLow:
                pushPCLow();
                break;
            
            case MicroOperation::PushAccumulator:
                pushAccumulator();
                break;
            
            case MicroOperation::PushStatusBRK:
                pushStatusBRK();
                break;
            
            case MicroOperation::DummyPush:
                dummyPush();
                break;

            case MicroOperation::DummyPushForceI:
                dummyPushForceI();
                break;
            
            case MicroOperation::PullStatus:
                pullStatus();
                break;

            case MicroOperation::PullPCLow:
                pullPCLow();
                break;

            case MicroOperation::PullPCHigh:
                pullPCHigh();
                break;
            
            case MicroOperation::PullAccumulator:
                pullAccumulator();
                break;

            case MicroOperation::PullStatusBRK:
                pullStatusBRK();
                break;

            case MicroOperation::FetchAddressHighToPC:
                fetchAddressHighToPC();
                break;

            case MicroOperation::FetchInterruptVectorHigh:
                fetchInterruptVectorHigh();
                break;

            case MicroOperation::FetchInterruptVectorLow:
                fetchInterruptVectorLow();
                break;

            case MicroOperation::FetchStartingLow:
                fetchStartingLow();
                break;
                
            case MicroOperation::FetchStartingHigh:
                fetchStartingHigh();
                break;

            case MicroOperation::StackPop:
                stackPop();
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
        opcode = bus->read(PC);
        incrementPC();
    }

    void MOS6502::fetchOpcodeAndDiscard()
    {
        data = bus->read(PC);
    }

    void MOS6502::fetchOpcodeDiscardAndExec()
    {
        opcode = bus->read(PC);
        executeInstruction();
    }

    void MOS6502::dummyRead()
    {
        uint16_t dummyAddress = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);

        data = bus -> read(dummyAddress);
        // Es importante conservarlo para instrucciones como AHX
        // addressHigh = static_cast<uint8_t> (address >> 8);
    }

    void MOS6502::dummyWrite()
    {
        bus->write(address, data);
    }

    void MOS6502::fetchOperand()
    {
        data = bus->read(PC);
        incrementPC();
        
        executeInstruction();
    }

    void MOS6502::readOperand()
    {
        data = bus -> read(address);
    }

    void MOS6502::writeOperand()
    {
        executeInstruction();
    }

    void MOS6502::readOperandAndExecute()
    {
        data = bus -> read(address);

        executeInstruction();
    }

    void MOS6502::readOperandVariable()
    {
        uint16_t speculativeAddress =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        data = bus->read(speculativeAddress);

        if(address == speculativeAddress)
        {
            executeInstruction();
            return;
        }

        addressHigh = static_cast<uint8_t>(addressHigh >> 8);
        currentInstruction->insertNext(MicroOperation::ReadOperandAndExecute);

    }

    void MOS6502::fetchZeroPageAddress()
    {
        addressHigh = 0;
        addressLow = bus -> read(PC);
        incrementPC();

        address = static_cast<uint16_t>(addressLow);
    }
    
    void MOS6502::fetchAddressLow()
    {
        addressLow = bus -> read(PC);
        incrementPC();

        address =
            static_cast<uint16_t>(
                (address & 0xFF00) |
                static_cast<uint8_t>(address + 1)
            );
    }
    void MOS6502::fetchAddressHigh()
    {
        addressHigh = bus -> read(PC);
        incrementPC();

        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
    }

    void MOS6502::fetchAddresshighExecute()
    {
        addressHigh = bus -> read(PC);
        incrementPC();

        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        executeInstruction();
    }

    void MOS6502::fetchAddressHighAddX()
    {
        fetchAddressHigh();

        uint16_t lowerSum =
            static_cast<uint16_t>(addressLow) +
            static_cast<uint16_t>(X);

        bool pageCrossed = (lowerSum & 0x100) != 0;

        addressLow = static_cast<uint8_t>(lowerSum & 0xFF);

        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh + pageCrossed) << 8);
    }

    void MOS6502::fetchAddressHighAddY()
    {
        fetchAddressHigh();

        uint16_t lowerSum =
            static_cast<uint16_t>(addressLow) +
            static_cast<uint16_t>(Y);

        bool pageCrossed = (lowerSum & 0x100) != 0;

        addressLow = static_cast<uint8_t>(lowerSum & 0xFF);

        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh + pageCrossed) << 8);
    }

    void MOS6502::fetchIndirectLow()
    {
        addressLow = bus->read(address);

        address =
            static_cast<uint16_t>(
                (address & 0xFF00) |
                static_cast<uint8_t>(address + 1)
            );
    }
    
    void MOS6502::fetchIndirectHigh()
    {
        addressHigh = bus -> read(address);
        
        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
    }

    void MOS6502::fetchIndirectHighAddY()
    {
        fetchIndirectHigh();

        uint16_t lowerSum =
            static_cast<uint16_t>(addressLow) +
            static_cast<uint16_t>(Y);

        bool pageCrossed = (lowerSum & 0x100) != 0;

        addressLow = static_cast<uint8_t>(lowerSum & 0xFF);

        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh + pageCrossed) << 8);
    }

    void MOS6502::fetchIndirectHighExecute()
    {
        fetchIndirectHigh();
        executeInstruction();
    }

    void MOS6502::addZeroPageAddress(uint8_t index)
    {
        addressLow =
            static_cast<uint8_t>(addressLow + index);

        addressHigh = 0x00;

        address =
            static_cast<uint16_t>(addressLow);
    }

    void MOS6502::addXToZeroPageAddress()
    {
        addZeroPageAddress(X);
    }
    void MOS6502::addYToZeroPageAddress()
    {
        addZeroPageAddress(Y);
    }

    void MOS6502::fetchBranch()
    {
        data = bus->read(PC);
        incrementPC();

        address = PC;

        addressHigh = static_cast<uint8_t>(PC >> 8);
        addressLow = static_cast<uint8_t>(PC & 0x00FF);

        executeInstruction();
    }

    void MOS6502::evaluateBranch()
    {

        int16_t offset = static_cast<int8_t>(data);

        uint16_t target =
            static_cast<uint16_t>(PC + offset);

        bool pageCrossed =
            (target & 0xFF00) != (PC & 0xFF00);

        address = target;

        // Primer ciclo adicional: dummy read del PC actual.
        data = bus->read(PC);

        if (!pageCrossed)
        {
            PC = target;
            return;
        }

        addressHigh = static_cast<uint8_t>(target >> 8);
        addressLow  = static_cast<uint8_t>(target & 0xFF);

        currentInstruction->insertNext(
            MicroOperation::DummyBranchPageCrossRead
        );
    }

    void MOS6502::dummyBranchPageCrossRead()
    {
        PC = address;
        data = bus->read(address);
    }

    void MOS6502::pushStatus()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        StatusRegister pushReg;
        pushReg.setByte(status.getByte());

        pushReg.setFlag(StatusRegister::B, 1);
        pushReg.setFlag(StatusRegister::U, 1);
        
        bus->write(stackAddress, pushReg.getByte());
        decrementS();
    }

    void MOS6502::pushPCHigh()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );
        
        uint8_t pushData =
            static_cast<uint8_t>(PC >> 8);
        

        bus->write(stackAddress, pushData);
        decrementS();
    }

    void MOS6502::pushPCLow()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );
        
        uint8_t pushData =
            static_cast<uint8_t>(PC & 0x00FF);
        

        bus->write(stackAddress, pushData);
        decrementS();
    }

    void MOS6502::pushAccumulator()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        bus->write(stackAddress, A);
        decrementS();
    }

    void MOS6502::pushStatusBRK()
    {
        StatusRegister pushReg;
        pushReg.setByte(status.getByte());

        pushReg.setFlag(StatusRegister::B, 1);
        pushReg.setFlag(StatusRegister::U, 1);

        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );
        
        bus->write(stackAddress,pushReg.getByte());
        decrementS();

        status.setFlag(StatusRegister::I, 1);
    }

    void MOS6502::dummyPush()
    {
        decrementS();
    }

    void MOS6502::dummyPushForceI()
    {
        decrementS();
        status.setFlag(StatusRegister::I, 1);
    }

    void MOS6502::pullStatusBRK()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        StatusRegister pullReg;
        pullReg.setByte(bus->read(stackAddress));

        pullReg.setFlag(
            StatusRegister::B,
            status.getFlag(StatusRegister::B)
        );

        pullReg.setFlag(
            StatusRegister::U,
            status.getFlag(StatusRegister::U)
        );

        status.setByte(pullReg.getByte());
        incrementS();
    }

    void MOS6502::pullPCLow()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        addressLow = bus->read(stackAddress);

        incrementS();
    }
    
    void MOS6502::pullPCHigh()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        addressHigh = bus->read(stackAddress);
        
        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        PC = address;
    }

    void MOS6502::pullAccumulator()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        A = bus->read(stackAddress);
        status.setFlag(StatusRegister::Z, A == 0);
        status.setFlag(StatusRegister::N, (A & 0x80) != 0);
    }

    void MOS6502::pullStatus()
    {
        uint16_t stackAddress =
            static_cast<uint16_t>(
                0x0100 | 
                static_cast<uint16_t>(S)
            );

        StatusRegister pullReg;
        pullReg.setByte(bus->read(stackAddress));

        pendingIUpdates = 1;
        iUpdate = pullReg.getFlag(StatusRegister::I);

        pullReg.setFlag(
            StatusRegister::B,
            status.getFlag(StatusRegister::B)
        );

        pullReg.setFlag(
            StatusRegister::U,
            status.getFlag(StatusRegister::U)
        );

        pullReg.setFlag(
            StatusRegister::I,
            status.getFlag(StatusRegister::I)
        );

        status.setByte(pullReg.getByte());

    }

    void MOS6502::fetchAddressHighToPC()
    {
        addressHigh = bus->read(PC);
        incrementPC();
        
        address =
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        PC = address;
    }

    void MOS6502::stackPop()
    {
        incrementS();
    }

    void MOS6502::fetchInterruptVectorLow()
    {
        address = 0xFFFE;
        addressLow = bus->read(address);
    }

    void MOS6502::fetchInterruptVectorHigh()
    {
        address = 0xFFFF;
        addressHigh = bus->read(address);

        PC = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
    }

    void MOS6502::fetchStartingLow()
    {
        address = 0xFFFC;
        addressLow = bus->read(address);
    }

    void MOS6502::fetchStartingHigh()
    {
        address = 0xFFFD;
        addressHigh = bus->read(address);

        PC = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
    }

    // Instrucciones

    // Instrucciones Legales

    void MOS6502::executeLDA()
    {
        A = data;
        status.setFlag(StatusRegister::Z, (data == 0));
        status.setFlag(StatusRegister::N, (data & (1<<7)) != 0);
    }
    void MOS6502::executeLDX()
    {
        X = data;
        status.setFlag(StatusRegister::Z, (data == 0));
        status.setFlag(StatusRegister::N, (data & (1<<7)) != 0);
    }
    void MOS6502::executeLDY()
    {
        Y = data;
        status.setFlag(StatusRegister::Z, (data == 0));
        status.setFlag(StatusRegister::N, (data & (1<<7)) != 0);
    }
    
    void MOS6502::executeSTA()
    {
        bus -> write(address, A);
    }
    void MOS6502::executeSTX()
    {
        bus -> write(address, X);
    }
    void MOS6502::executeSTY()
    {
        bus -> write(address, Y);
    }
    
    void MOS6502::executeTAX()
    {
        X = A;
    }
    void MOS6502::executeTAY()
    {
        Y = A;
    }
    void MOS6502::executeTXA()
    {
        A = X;
    }
    void MOS6502::executeTYA()
    {
        A = Y;
    }

    void MOS6502::executeADC()
    {
        ALUResult result = alu.add(A, data, status.getFlag(StatusRegister::C));
        data = A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeSBC()
    {
        ALUResult result = alu.substract(A, data, status.getFlag(StatusRegister::C));
        data = A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeINC()
    {
        ALUResult result = alu.increment(data);
        data = result.result;
        bus -> write(address, result.result);
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeDEC()
    {
        ALUResult result = alu.decrement(data);
        data = result.result;
        bus -> write(address, result.result);
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeINX()
    {
        ALUResult result = alu.increment(X);
        X = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeDEX()
    {
        ALUResult result = alu.decrement(X);
        X = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeINY()
    {
        ALUResult result = alu.increment(Y);
        Y = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeDEY()
    {
        ALUResult result = alu.decrement(Y);
        Y = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }

    void MOS6502::executeASL()
    {
        if(currentInstruction->getAddressingMode() == AddressingMode::Accumulator)
        {
            ALUResult result = alu.shiftLeft(A);
            data = A = result.result;
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
        else
        {
            ALUResult result = alu.shiftLeft(data);
            data = result.result;
            bus -> write(address, result.result);
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
    }
    void MOS6502::executeLSR()
    {
        if(currentInstruction->getAddressingMode() == AddressingMode::Accumulator)
        {
            ALUResult result = alu.shiftRight(A);
            data = A = result.result;
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
        else
        {
            ALUResult result = alu.shiftRight(data);
            data = result.result;
            bus -> write(address, result.result);
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
    }
    void MOS6502::executeROL()
    {
        if(currentInstruction->getAddressingMode() == AddressingMode::Accumulator)
        {
            ALUResult result = alu.rotateLeft(A, status.getFlag(StatusRegister::C));
            data = A = result.result;
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
        else
        {
            ALUResult result = alu.rotateLeft(data, status.getFlag(StatusRegister::C));
            data = result.result;
            bus -> write(address, result.result);
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
    }
    void MOS6502::executeROR()
    {
        if(currentInstruction->getAddressingMode() == AddressingMode::Accumulator)
        {
            ALUResult result = alu.rotateRight(A, status.getFlag(StatusRegister::C));
            data = A = result.result;
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
        else
        {
            ALUResult result = alu.rotateRight(data, status.getFlag(StatusRegister::C));
            data = result.result;
            bus -> write(address, result.result);
            status.applyMask(result.statusBits, result.statusChangedBits);
        }
    }
    
    void MOS6502::executeAND()
    {
        ALUResult result = alu.andOp(A, data);
        data = A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeORA()
    {
        ALUResult result = alu.orOp(A, data);
        data = A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeEOR()
    {
        ALUResult result = alu.xorOp(A, data);
        data = A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeBIT()
    {
        ALUResult result = alu.bitTest(A, data);
        status.applyMask(result.statusBits, result.statusChangedBits);
    }

    void MOS6502::executeCMP()
    {
        ALUResult result = alu.compare(A, data);
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeCPX()
    {
        ALUResult result = alu.compare(X, data);
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeCPY()
    {
        ALUResult result = alu.compare(Y, data);
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    
    void MOS6502::executeBCC()
    {
        if(!status.getFlag(StatusRegister::C))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBCS()
    {
        if(status.getFlag(StatusRegister::C))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBEQ()
    {
        if(status.getFlag(StatusRegister::Z))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBNE()
    {
        if(!status.getFlag(StatusRegister::Z))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBPL()
    {
        if(!status.getFlag(StatusRegister::N))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBMI()
    {
        if(status.getFlag(StatusRegister::N))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBVC()
    {
        if(!status.getFlag(StatusRegister::V))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }
    void MOS6502::executeBVS()
    {
        if(status.getFlag(StatusRegister::C))
            currentInstruction->insertNext(
                MicroOperation::EvaluateBranch
            );
    }

    void MOS6502::executeJMP()
    {
        PC = address;
    }

    void MOS6502::executeTXS()
    {
        S = X;
    }
    void MOS6502::executeTSX()
    {
        X = S;
    }

    void MOS6502::executeCLC()
    {
        status.setFlag(StatusRegister::FlagID::C, 0);
    }
    void MOS6502::executeSEC()
    {
        status.setFlag(StatusRegister::FlagID::C, 1);
    }
    void MOS6502::executeCLI()
    {
        pendingIUpdates = 1;
        iUpdate = 0;
    }
    void MOS6502::executeSEI()
    {
        pendingIUpdates = 1;
        iUpdate = 1;
    }
    void MOS6502::executeCLD()
    {
        status.setFlag(StatusRegister::FlagID::D, 0);
    }
    void MOS6502::executeSED()
    {
        status.setFlag(StatusRegister::FlagID::D, 1);
    }
    void MOS6502::executeCLV()
    {
        status.setFlag(StatusRegister::FlagID::V, 0);
    }

    void MOS6502::executeNOP()
    {

    }

    // Instrucciones Ilegales

    void MOS6502::executeSLO()
    {
        executeASL();
        executeORA();
    }
    void MOS6502::executeRLA()
    {
        executeROL();
        executeAND();
    }
    void MOS6502::executeSRE()
    {
        executeLSR();
        executeEOR();
    }
    void MOS6502::executeRRA()
    {
        executeROR();
        executeADC();
    }
    void MOS6502::executeSAX()
    {
        ALUResult result = alu.andOp(A, X);
        bus->write(address, result.result);
    }
    void MOS6502::executeLAX()
    {
        X = data;
        A = data;

        status.setFlag(StatusRegister::Z, data==0);
        status.setFlag(StatusRegister::N, (data & (0x80) != 0));
    }
    void MOS6502::executeDCP()
    {
        executeDEC();
        executeCMP();
    }
    void MOS6502::executeISC()
    {
        bool saveC = status.getFlag(StatusRegister::C);
        executeINC();
        status.setFlag(StatusRegister::C, saveC);
        executeSBC();
    }

    void MOS6502::executeANC()
    {
        ALUResult result = alu.andOp(A, data);
        data = A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);

        status.setFlag(StatusRegister::C, status.getFlag(StatusRegister::N));
    }
    void MOS6502::executeALR()
    {
        executeAND();
        ALUResult result = alu.shiftRight(A);
        A = result.result;
        status.applyMask(result.statusBits, result.statusChangedBits);
    }
    void MOS6502::executeARR()
    {
        ALUResult result = alu.andOp(A, data);

        bool bit5 = (result.result & ((uint8_t)1 << 5)); 
        bool bit6 = (result.result & ((uint8_t)1 << 6));

        status.setFlag(StatusRegister::V, bit5 ^ bit6);
        status.setFlag(StatusRegister::C, bit6);

        ALUResult result2 = alu.shiftRight(A);
        A = result2.result;
        
        status.setFlag(StatusRegister::Z, A == 0);
        status.setFlag(StatusRegister::N, (A & (0x80) != 0));
    }
    void MOS6502::executeXAA()
    {
        // Puede ser 0x00, 0xFF o 0xEE
        uint8_t magicConstant = 0x00;
        uint8_t realValue = A | magicConstant;

        A = realValue & X & data;
        status.setFlag(StatusRegister::Z, A == 0);
        status.setFlag(StatusRegister::N, (A & 0x80) != 0);
    }
    void MOS6502::executeAXS()
    {
        ALUResult result = alu.andOp(A, data);
        ALUResult result2 = alu.substract(result.result, data, status.getFlag(StatusRegister::C));

        X = result2.result;

        ALUResult result3 = alu.compare(result.result, data);
        status.applyMask(result3.statusBits, result3.statusChangedBits);
    }

    void MOS6502::executeAHX()
    {
        uint8_t value = A & X & (static_cast<uint8_t>(addressHigh + 1));
        uint16_t unstableAddress = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        bus->write(unstableAddress, value);
    }
    void MOS6502::executeSHX()
    {
        uint8_t value = X & (static_cast<uint8_t>(addressHigh + 1));
        uint16_t unstableAddress = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        bus->write(unstableAddress, value);
    }
    void MOS6502::executeSHY()
    {
        uint8_t value = Y & (static_cast<uint8_t>(addressHigh + 1));
        uint16_t unstableAddress = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);
        
        bus->write(unstableAddress, value);
    }

    void MOS6502::executeTAS()
    {
        S = A & X;
        uint8_t value = S & (static_cast<uint8_t>(addressHigh + 1));

        uint16_t unstableAddress = 
            static_cast<uint16_t>(addressLow) |
            (static_cast<uint16_t>(addressHigh) << 8);

        bus->write(unstableAddress, value);
    }
    void MOS6502::executeLAS()
    {
        uint8_t value = data & S;

        A = X = S = value;

        status.setFlag(StatusRegister::Z, value == 0);
        status.setFlag(StatusRegister::N, (value & 0x80) != 0);
    }

    void MOS6502::executeKIL()
    {
        isJammed = 1;
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

        definition = instructionTable.get(0xE8);
        mockInstruction = instructionFactory.create(definition);

        mockInstruction.printInstruction();

        definition = instructionTable.get(0x18);
        mockInstruction = instructionFactory.create(definition);

        mockInstruction.printInstruction();
    }
}
