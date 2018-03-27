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
class physFile {
public:
	physFile() {}
	void init(uint64_t size){
		pregSize = size;
		pregFile = (pregEntry*) calloc(size,sizeof(pregEntry));
		numFree = (int)size;
	}

	int fill_first_preg(){
		for(int i=0; i<(int)pregSize; i++){
			if(pregFile[i].occupied == 0){
				pregFile[i].occupied = 1;
				pregFile[i].ready = 0;
				numFree -= 1;
				return i;
			}
		}
		return -1;
	}

	void clear_preg(int index){
		pregFile[index].occupied = 0;
		pregFile[index].ready = 0;
		numFree += 1;
	}

	void clear(){
		free(pregFile);
	}
	uint64_t pregSize;
	pregEntry* pregFile;
	int numFree;
private:
};

class state_update {
public:
	state_update() {}
	void init(uint64_t size){
		robSize = size;
		robEnd = (int) size - 1;
		rob_ = (robEntry*) calloc(size,sizeof(robEntry));
		numFree = (int) size;
	}

	void update(physFile& preg){
		int freeable = -1;
		while((rob_[robEnd].busy == 0) && (rob_[robEnd].occupied == 1)){
			freeable = this -> shiftForward();
			if(freeable != -1){
				preg.clear_preg(freeable);
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

	void add_entry(robEntry * newEntry){
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
	
	void mark_complete(int id){
		for(int i=0; i<(int)robSize; i++){
			if(rob_[i].id == id){
				rob_[i].busy = 0;
			}
		}
	}

	void clear(){
		free(rob_);
		//rob_ = NULL;
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
	void init(uint64_t size, uint64_t k0, uint64_t k1, uint64_t k2){
		schedSize = size;
		schedQ = (schedEntry*) calloc(size,sizeof(schedEntry));
		numFree = (int)size;
		k_avail[0]=(int)k0; k_avail[1]=(int)k1; k_avail[3]=(int)k2;
	}

	bool add_entry(schedEntry * newEntry){
		for(int i=0; i<(int)schedSize; i++){
			if(schedQ[i].occupied == 0){
				schedQ[i] = *newEntry;
				//printf("schedQ entries: ");
				//for(int j=0;j<(int)schedSize;j++){
				//	printf("%d ",schedQ[j].occupied);
				//}
				//printf("\n");
				numFree -= 1;
				return true;
			}
		}
		return false;
	}

	void complete_instructions(state_update& SU){
		for(int i=0; i<(int)schedSize; i++){
			uint32_t num;
			if((schedQ[i].occupied == 1) && (schedQ[i].marked_to_fire == 1)){
				schedQ[i].occupied = 0;
				schedQ[i].marked_to_fire = 0;
				num = schedQ[i].id;
				SU.mark_complete(num);
				numFree += 1;
			}
		}
	}

	void mark_to_execute(){

	}

	void clear(){
		free(schedQ);
	}

	int numFree;
	uint64_t schedSize;
	schedEntry* schedQ;
	int k_avail[3];
private:
};



//class execute{
//public:
//	execute(){}
//	void init(uint64_t k0, uint64_t k1, uint64_t k2){
//		uint64_t k[3];
//		k[0]=k0; k[1]=k1; k[2]=k2;
//		fus = (function_unit**) calloc(0,3*sizeof(function_unit*));
//		for(int i=0; i<3; i++){
//			fus[i] = (function_unit*) calloc(0,k[i]*sizeof(function_unit));
//		}
//	}
//	function_unit ** fus;
//private:
//};

//std::vector<robEntry> rob;
state_update SU;
const int numAregs = 32;
int32_t rat[numAregs];
pregEntry* pregFile;
//robEntry* rob;
uint64_t numk0, numk1, numk2, numPreg, dispRate, robSize, schedSize;

scheduler_ scheduler;
physFile pregs;
//dispatcher disp;
//execute ex;
int instNum = 0;
int clock = 0;

void setup_proc(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t rob_, uint64_t preg) 
{
	numk0 = k0; numk1 = k1; numk2 = k2; 
	dispRate = f; robSize = rob_; numPreg = preg;
	schedSize = 2*k0 + k1 + k2;
	//pregFile = (pregEntry*) calloc(0,sizeof(pregEntry) * numPreg);
	///rob = (robEntry*) calloc(0,sizeof(robEntry) * robSize);
	//rob.reserve(robSize);
	//printf("0");
	SU.init(robSize);
	//printf("1");
	scheduler.init(schedSize,k0,k1,k2);
	//printf("2");
	pregs.init(numPreg);
	//printf("3");
	for(int i=0;i<numAregs;i++){
		rat[i]=-1;
	}
	//ex.init(k0,k1,k2);
	//disp.init(dispRate);
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
robEntry newrob;
schedEntry newsched;
int notFinalInst = 1;

void dispatch(){
	int num_to_dispatch = (int)dispRate;
	if(SU.numFree < num_to_dispatch){
		num_to_dispatch = SU.numFree;
		//printf("%d ",SU.numFree);
	}
	if(scheduler.numFree < num_to_dispatch){
		num_to_dispatch = scheduler.numFree;
		//printf("%d ",scheduler.numFree);
	}
	if(pregs.numFree < num_to_dispatch){
		num_to_dispatch = pregs.numFree;
		//printf("%d\n",pregs.numFree);
	}
	int i=0;
	//printf("xd");
	while((i < num_to_dispatch) && (notFinalInst = read_instruction(&instruction))){
		//printf("%d ",SU.numFree);printf("%d ",scheduler.numFree);printf("%d\n",pregs.numFree);
		//printf("%d\n",num_to_dispatch);
		//printf("%d",notFinalInst);
		i++;
		instNum++;
		//SU.addEntry(&newentry);
		int32_t opcode = instruction.op_code;
		if(opcode == -1){
			opcode = 1;
		}
		//uint32_t address = instruction.instruction_address;
		int32_t src1 = instruction.src_reg[0];
		int32_t src2 = instruction.src_reg[1];
		int32_t dest = instruction.dest_reg;
		//printf("%d\n", opcode);
		int32_t preg1 = rat[src1];
		int32_t preg2 = rat[src2];
		int32_t prev = rat[dest];
		int32_t preg = -1;
		if(dest > 0){
			preg = pregs.fill_first_preg();
			rat[dest] = preg;
		}
		newrob = {.occupied = 1, .busy = 1, .areg = dest, .prevPreg = prev, .preg = preg, .id = instNum};
		newsched = {.occupied = 1, .marked_to_fire = 0, .id = instNum, .dest_areg = dest, .dest_preg = preg, .src1_preg = preg1, .src2_preg = preg2, .FU=opcode};
	
		SU.add_entry(&newrob);
		scheduler.add_entry(&newsched);
		//printf("%d\n",instNum);
	}

}

void run_proc(proc_stats_t* p_stats){
	while((notFinalInst == 1) || (SU.numFree < (int)robSize)){
		clock++;
		SU.update(pregs);
		scheduler.complete_instructions(SU);
		scheduler.mark_to_execute();
		dispatch();
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
	SU.clear();
	scheduler.clear();
	pregs.clear();
}
