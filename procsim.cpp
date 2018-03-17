#include "procsim.hpp"
#include <vector>
/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @k0 Number of k0 FUs
 * @k1 Number of k1 FUs
 * @k2 Number of k2 FUs
 * @f Number of instructions to fetch
 * @rob Number of ROB entries
 * @preg Number of physical registers
 */
typedef struct _preg{
	int ready;
} pregEntry;

typedef struct _robEntry{
	int32_t areg;
	int32_t prevPreg;
	int32_t preg;
} robEntry;

class scheduler{
public:
	scheduler(){
		dest_areg = 0;
		dest_preg = 1;
		src1_preg = 2;
		src2_preg = 3;
	}
	int32_t dest_areg;
	int32_t dest_preg;
	int32_t src1_preg;
	int32_t src2_preg;
};

std::vector<robEntry> rob;

int rat[32];
pregEntry* pregFile;
//robEntry* rob;
uint64_t numk0, numk1, numk2, numPreg, dispRate, robSize, schedSize;

scheduler* sched;

void setup_proc(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t rob_, uint64_t preg) 
{
	numk0 = k0; numk1 = k1; numk2 = k2; dispRate = f; robSize = rob_; numPreg = preg;
	schedSize = 2*k0 + k1 + k2;
	pregFile = (pregEntry*) calloc(0,sizeof(pregEntry) * numPreg);
	///rob = (robEntry*) calloc(0,sizeof(robEntry) * robSize);
	rob.reserve(robSize);
	sched = (scheduler*) malloc(sizeof(scheduler) * schedSize);
	//printf("%d",sizeof(scheduler));
	printf("%d,%d\n",sched[0].dest_areg,sched[0].dest_preg);
}

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
proc_inst_t instruction;

void run_proc(proc_stats_t* p_stats){

	while(read_instruction(&instruction)){
		int32_t opcode = instruction.op_code;
		if(opcode = -1){
			opcode = 1;
		}
		uint32_t address = instruction.instruction_address;
		int32_t src1 = instruction.src_reg[0];
		int32_t src2 = instruction.src_reg[1];
		int32_t dest = instruction.dest_reg;
		//printf("%d\n", opcode);

	}
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC, average fire rate etc.
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) 
{

}
