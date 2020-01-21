#pragma once

#include "platform.h"

typedef struct sw_vm sw_vm;

struct sw_vm {
	int halt;
};

void sw_halt(sw_vm *vm);

static sw_forceinline void
sw_checkpoint(sw_vm *vm)
{
	if (vm->halt) sw_halt(vm);
}
