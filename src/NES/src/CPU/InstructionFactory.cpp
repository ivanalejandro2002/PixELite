#include <NES/CPU/Instruction/InstructionFactory.h>

namespace NES::CPU
{
    Instruction InstructionFactory::create(
        const InstructionDefinition& definition
    ) const
    {
        InstructionBuilder builder;

        switch (definition.operation)
        {
            case InstructionOperation::JSR:
                return buildJSR();

            case InstructionOperation::RTS:
                return buildRTS();

            case InstructionOperation::BRK:
                return buildBRK();

            case InstructionOperation::RTI:
                return buildRTI();
            
            case InstructionOperation::PHA:
                return buildPHA();

            case InstructionOperation::PLA:
                return buildPLA();

            case InstructionOperation::PHP:
                return buildPHP();

            case InstructionOperation::PLP:
                return buildPLP();

            case InstructionOperation::TurnOn:
                return buildTurnOn();
                break;
            
            case InstructionOperation::Reset:
                return buildReset();
                break;

            default:
                switch(definition.accessType)
                {
                    case AccessType::None:
                        AddressingModes::build(
                            definition.addressingMode,
                            builder
                        );
                        break;
                    case AccessType::Read:
                        AddressingModes::buildRead(
                            definition.addressingMode,
                            builder
                        );
                        break;
                    case AccessType::Write:
                        AddressingModes::buildWrite(
                            definition.addressingMode,
                            builder
                        );
                        break;
                    case AccessType::ReadModifyWrite:
                        AddressingModes::buildRMW(
                            definition.addressingMode,
                            builder
                        );
                        break;
                }
                

                return builder.build(
                    definition.addressingMode,
                    definition.operation,
                    definition.accessType
                );
        }
    }

    Instruction InstructionFactory::buildJSR() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchZeroPageAddress)
            .add(MicroOperation::DummyRead)
            .add(MicroOperation::PushPCHigh)
            .add(MicroOperation::PushPCLow)
            .add(MicroOperation::FetchAddressHighToPC);

        return builder.build(
            AddressingMode::Absolute,
            InstructionOperation::JSR,
            AccessType::None
        );
    }

    Instruction InstructionFactory::buildRTS() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcode)
            .add(MicroOperation::StackPop)
            .add(MicroOperation::PullPCLow)
            .add(MicroOperation::PullPCHigh)
            .add(MicroOperation::FetchOpcode);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::RTS,
            AccessType::None
        );
    }

    Instruction InstructionFactory::buildBRK() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcode)
            .add(MicroOperation::PushPCHigh)
            .add(MicroOperation::PushPCLow)
            .add(MicroOperation::PushStatusBRK)
            // $FFFE
            .add(MicroOperation::FetchInterruptVectorLow)
            // $FFFF
            .add(MicroOperation::FetchInterruptVectorHigh);


        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::BRK,
            AccessType::None
        );
    }

    Instruction InstructionFactory::buildRTI() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcode)
            .add(MicroOperation::StackPop)
            .add(MicroOperation::PullStatusBRK)
            .add(MicroOperation::PullPCLow)
            .add(MicroOperation::PullPCHigh);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::RTI,
            AccessType::None
        );
    }

    Instruction InstructionFactory::buildPHA() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcodeAndDiscard)
            .add(MicroOperation::PushAccumulator);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::PHA,
            AccessType::Write
        );
    }

    Instruction InstructionFactory::buildPLA() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcodeAndDiscard)
            .add(MicroOperation::StackPop)
            .add(MicroOperation::PullAccumulator);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::PLA,
            AccessType::Read
        );
    }

    Instruction InstructionFactory::buildPHP() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcodeAndDiscard)
            .add(MicroOperation::PushStatus);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::PHP,
            AccessType::Write
        );
    }

    Instruction InstructionFactory::buildPLP() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcodeAndDiscard)
            .add(MicroOperation::StackPop)
            .add(MicroOperation::PullStatus);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::PLP,
            AccessType::Read
        );
    }

    Instruction InstructionFactory::buildReset() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcode)
            .add(MicroOperation::DummyPush)
            .add(MicroOperation::DummyPush)
            .add(MicroOperation::DummyPushForceI)
            .add(MicroOperation::FetchStartingLow)
            .add(MicroOperation::FetchStartingHigh);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::Reset,
            AccessType::None
        );
    }

    Instruction InstructionFactory::buildTurnOn() const
    {
        InstructionBuilder builder;

        builder
            .add(MicroOperation::FetchOpcodeAndDiscard)
            .add(MicroOperation::DummyPush)
            .add(MicroOperation::DummyPush)
            .add(MicroOperation::DummyPushForceI)
            .add(MicroOperation::FetchStartingLow)
            .add(MicroOperation::FetchStartingHigh);

        return builder.build(
            AddressingMode::Implied,
            InstructionOperation::TurnOn,
            AccessType::None
        );
    }
}