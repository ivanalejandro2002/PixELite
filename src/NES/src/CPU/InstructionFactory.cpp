#include <NES/CPU/Instruction/InstructionFactory.h>

namespace NES::CPU
{
    Instruction InstructionFactory::create(
        const InstructionDefinition& definition
    ) const
    {
        InstructionBuilder builder;

        AddressingModes::build(
            definition.addressingMode,
            builder
        );

        builder.add(MicroOperation::Execute);

        return builder.build(definition.operation);
    }
}