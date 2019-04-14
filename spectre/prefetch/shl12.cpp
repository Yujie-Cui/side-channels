/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdio.h>
#include <string>
#include <unordered_map>

#include "pin.H"

FILE * trace;
static std::tr1::unordered_map<ADDRINT, std::string> ins_str;
static unsigned int total_insn = 0;
static unsigned int counter = 0;

VOID insn_count() {
    total_insn++;
	//printf("%p: %s\n", ip, ins_str[addr].c_str());
}

VOID test(VOID* ip, ADDRINT addr) {
    counter++;
	//printf("%p: %s\n", ip, ins_str[addr].c_str());
}

// Is called for every instruction and instruments reads and writes
VOID Instruction(INS ins, VOID *v) {

	INS_InsertPredicatedCall(
	ins, IPOINT_BEFORE, (AFUNPTR)insn_count,
	IARG_END);

	// create ip -> string mapping
	ADDRINT addr = INS_Address(ins);
	ins_str[addr] = INS_Disassemble(ins);

	if(INS_Mnemonic(ins) == "SHL") {
		
		INT32 num_ops = INS_OperandCount(ins);

		for(INT32 i=0; i<num_ops; i++) {
			
			if (INS_OperandIsImmediate(ins, i)) {
				UINT64 shift_val = INS_OperandImmediate(ins, i);
				if(shift_val == 0xc) { // shift by 0xc = shift by 12 = multiply by 4096
					INS_InsertPredicatedCall(
					ins, IPOINT_BEFORE, (AFUNPTR)test,
					IARG_INST_PTR,
					IARG_ADDRINT, addr,
					IARG_END);
				}
			} // is immediate op ; end

		} // iterate through all ops ; end
	
	} // is shift insn ; end
}

VOID Fini(INT32 code, VOID *v) {
    printf("Shift by 0xc: %u times (%.2f%%)\n", counter, (float)counter/total_insn * 100);
	fprintf(trace, "#eof\n");
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */
   
INT32 Usage() {
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[]) {

    if (PIN_Init(argc, argv)) return Usage();

    trace = fopen("spectre.out", "w");

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();
    
    return 0;
}
