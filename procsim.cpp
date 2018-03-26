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
	int busy;
} pregEntry;

typedef struct _robEntry{
	int occupied;
	int32_t busy;
	int32_t areg;
	int32_t prevPreg;
	int32_t preg;
	uint32_t instruction_address;
} robEntry;

typedef struct _schedEntry{
	int32_t busy;
	uint32_t instruction_address;
	int32_t dest_areg;
	int32_t dest_preg;
	int32_t src1_preg;
	int32_t src2_preg;
} schedEntry;

class state_update {
public:
	state_update() {}
	void init(uint64_t size){
		robSize = size;
		robEnd = (int) size - 1;
		rob_ = (robEntry*) calloc(0,sizeof(robEntry) * size);
		printf("rob created\n");
		numFree = (int) size;
	}

	void update(pregEntry* regFile){
		int freeable = -1;
		while(rob_[robEnd].busy == 0){
			freeable = this -> shiftForward();
			if(freeable != -1){
				regFile[freeable].busy = 0;
			}
			if(rob_[robEnd].occupied == 1){
				numFree += 1;
			}
		}
	}

	int shiftForward(){
		int preg_to_free = -1;
		if((rob_[robEnd].prevPreg != rob_[robEnd].preg) && (rob_[robEnd].busy == 0) && (rob_[robEnd].occupied == 1)){
			preg_to_free = rob_[robEnd].prevPreg;
		}
		for(int i=robEnd; i>0; i--){
			rob_[i] = rob_[i-1];
		}
		rob_[0].busy = 0;
		rob_[0].occupied = 0;
		return preg_to_free;
	}

	void addEntry(robEntry * newEntry){
		int highestFree = -1;
		for(int i=0; i<((int)robSize); i++){
			if(rob_[i].occupied == 0){
				if(i > highestFree){
					highestFree = i;
				}
			}
		}
		rob_[highestFree] = *newEntry;
		rob_[highestFree] = {.occupied = 1, .busy = 1};
		numFree -= 1;
	}
	
	robEntry* rob_;
	uint64_t robSize;
	int robEnd;
	int numFree;
private:
};

class scheduler_ {
public:
	scheduler_() {}
	void init(uint64_t size){
		schedSize = size;
		schedQ = (schedEntry*) calloc(0,sizeof(schedEntry) * size);
		printf("scheduling queue created\n");
	}
	uint64_t schedSize;
	schedEntry* schedQ;
private:
};

class physFile {
public:
	physFile() {}
	void init(uint64_t size){
		pregSize = size;
		pregFile = (pregEntry*) calloc(0,sizeof(pregEntry) * size);
		printf("preg file created\n");
	}
	uint64_t pregSize;
	pregEntry* pregFile;
private:
};

//std::vector<robEntry> rob;
state_update rob;
const int numAregs = 32;
int rat[numAregs];
pregEntry* pregFile;
//robEntry* rob;
uint64_t numk0, numk1, numk2, numPreg, dispRate, robSize, schedSize;

scheduler_ scheduler;
physFile pregs;

void setup_proc(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t rob_, uint64_t preg) 
{
	numk0 = k0; numk1 = k1; numk2 = k2; dispRate = f; robSize = rob_; numPreg = preg;
	schedSize = 2*k0 + k1 + k2;
	//pregFile = (pregEntry*) calloc(0,sizeof(pregEntry) * numPreg);
	///rob = (robEntry*) calloc(0,sizeof(robEntry) * robSize);
	//rob.reserve(robSize);
	rob.init(robSize);
	scheduler.init(schedSize);
	pregs.init(numPreg);
	//printf("%d",sizeof(scheduler));
	//printf("%d,%d\n",sched[0].dest_areg,sched[0].dest_preg);
}

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
proc_inst_t instruction;
robEntry newentry;
void run_proc(proc_stats_t* p_stats){

	while(read_instruction(&instruction)){
		rob.addEntry(&newentry);
		int32_t opcode = instruction.op_code;
		if(opcode == -1){
			opcode = 1;
		}
		uint32_t address = instruction.instruction_address;
		int32_t src1 = instruction.src_reg[0];
		int32_t src2 = instruction.src_reg[1];
		int32_t dest = instruction.dest_reg;
		//printf("%d\n", opcode);
		rob.update(pregs.pregFile);

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
