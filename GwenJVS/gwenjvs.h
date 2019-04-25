#ifndef GWENJVS_GWENJVS_H
#define GWENJVS_GWENJVS_H

#define WIN32_LEAN_AND_MEAN

#include <cstdio>
#include <cstdint>
#include <string>
#include <memory>
#include <windows.h>

#include "input.h"
#include "output.h"
#include "helpers.h"
#include "config.h"

enum kOutput {
	kOutputKb,
	kOutputXInput,
	kOutputDInput
};

#endif