#include "procsim.hpp"
#include "limits.h"
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
	void init(uint64_t size){ //create physical register file, record size and free space
		pregSize = size;
		pregFile = (pregEntry*) calloc(size,sizeof(pregEntry));
		numFree = (int)size;
	}

	int fill_first_preg(){ //find the first free preg and return the index
		for(int i=0; i<(int)pregSize; i++){
			if(pregFile[i].occupied == 0){ //if the preg is not currently occupied
				pregFile[i].occupied = 1; //set it to be occupied
				pregFile[i].ready = 0; //set it to be not ready (because it is only assigned when an instruction with it as an output is dispatched)
				numFree -= 1; //update number of free registers
				return i; //return the index
			}
		}
		return -1;
	}

	void clear_preg(int index){ //free a preg
		pregFile[index].occupied = 0; //set occupied bit to 0
		pregFile[index].ready = 0;
		numFree += 1; //update number of free registers
	}

	void ready_preg(int32_t index){ //sets a preg as ready
		pregFile[index].ready = 1; //set the bit to ready
	}

	void clear(){
		free(pregFile); //free memory
	}
	bool isReady(int32_t index){ //check if a preg is ready
		if(index > -1){
			bool value = ((pregFile[index].ready == 1) && (pregFile[index].occupied == 1));
			return value; //return true if the preg is both ready and occupied
		}
		else return true; //if the index entered is -1, it is still mapped to the architected register, meaning the instruction can fire
	}
	uint64_t pregSize;
	pregEntry* pregFile;
	int numFree;
private:
};

class state_update {
public:
	state_update() {}
	void init(uint64_t size){ //allocate memory for ROB, record the size and number of free entires
		robSize = size;
		robEnd = (int) size - 1;
		rob_ = (robEntry*) calloc(size,sizeof(robEntry));
		numFree = (int) size;
	}

	void update(physFile& preg, scheduler_& scheduler){ //if the head of the rob is ready to be retired, do so
		int freeable = -1;
		while((rob_[robEnd].busy == 0) && (rob_[robEnd].occupied == 1)){ //while the head of the rob can be retired (no limit to how many can be retired)
			scheduler.remove(rob_[robEnd].id); //remove the entry from the scheduler
			numFree += 1; //increase the number of free entries
			// UNCOMMENT FOR DEBUG
			//printf("RU Inst: %d \n",rob_[robEnd].id);
			freeable = this -> shiftForward(); //shift forward, see which preg can be freed
			if(freeable != -1){
				preg.clear_preg(freeable); //if a preg can be freed, free it
			}
		}
		// print rob contents
		/*
		printf("rob entries: ");
		for(int j=0;j<(int)robSize;j++){
			printf("%d/%d/%d ",rob_[j].id,rob_[j].occupied,rob_[j].busy);
		}
		printf("\n");
		printf("rob free:%d\n",numFree);
		*/
	}

	int shiftForward(){
		int preg_to_free = -1;
		if((rob_[robEnd].busy == 0) && (rob_[robEnd].occupied == 1)){
			preg_to_free = rob_[robEnd].prevPreg; //record what preg can be freed if one can be freed
		}
		for(int i=robEnd; i>0; i--){
			rob_[i] = rob_[i-1]; //shift entire rob forward
		}
		rob_[0].busy = 1;
		rob_[0].occupied = 0; //set the tail of the rob empty
		return preg_to_free; //return the index of the freeable preg
	}

	void add_entry(robEntry * newEntry){ //add an entry
		int highestFree = -1;
		for(int i=0; i<((int)robSize); i++){
			if(rob_[i].occupied == 0){
				if(i > highestFree){
					highestFree = i; //find the highest free slot
				}
			}
		}
		rob_[highestFree] = *newEntry; //add the entry to that slot
		numFree -= 1;
	}
	
	void mark_complete(int id){ //mark an instruction complete
		for(int i=0; i<(int)robSize; i++){
			if(rob_[i].id == id){
				rob_[i].busy = 0; //find the instruction in the rob and set it as not busy (ready to be retired)
			}
		}
	}

	void clear(){
		free(rob_); //free memory
	}

	robEntry* rob_;
	uint64_t robSize;
	int robEnd;
	int numFree;
private:
};


scheduler_::scheduler_() {}
void scheduler_::init(uint64_t size, uint64_t k0, uint64_t k1, uint64_t k2){
	schedSize = size; //store the size
	schedQ = (schedEntry*) calloc(size,sizeof(schedEntry)); //allocate scheduling queue, set to 0
	numFree = (int)size; //store number of free slots
	k_avail[0]=(int)k0; k_avail[1]=(int)k1; k_avail[2]=(int)k2; //store number of function units
}

bool scheduler_::add_entry(schedEntry * newEntry){ //add new entry
	for(int i=0; i<(int)schedSize; i++){
		if(schedQ[i].occupied == 0){ //search for next free slot
			schedQ[i] = *newEntry; //add new entry to queue
			numFree -= 1; //update the count of free slots
			return true;
		}
	}
	return false;
}

void scheduler_::remove(int id){ //remove the entry with the matching id
	for(int i=0; i<(int)schedSize; i++){
		if((schedQ[i].id == id)){
			schedQ[i].occupied = 0; //set occupied to 0, flagging it for replacement
			numFree += 1; //uupdate number of free slots
		}
	}
}

void scheduler_::sort_by_id(){ //bubble sort to sort scheduler entries by id to print properly
	int i,j;
	schedEntry temp;
	for(i=0;i<((int)schedSize - 1);i++){
		for(j=0;j<((int)schedSize - i - 1);j++){
			if(schedQ[j].id > schedQ[j+1].id){
				temp = schedQ[j];
				schedQ[j]=schedQ[j+1];
				schedQ[j+1]=temp;
			}
		}
	}
}

void scheduler_::complete_instructions(state_update& SU, physFile& preg){
	this -> sort_by_id(); //this is only necessary to make the outputs match the debugger
	for(int32_t unit=0;unit<3;unit++){
		for(int i=0; i<(int)schedSize; i++){
			if((schedQ[i].occupied == 1) && (schedQ[i].marked_to_fire == 1) && (schedQ[i].FU==unit))
			{
				schedQ[i].completed = 1; //mark entry as completed
				schedQ[i].marked_to_fire = 0; //no longer marked to fire
				if(schedQ[i].dest_preg > -1){
					preg.ready_preg(schedQ[i].dest_preg); //ready the preg to be used as input
				}
				SU.mark_complete(schedQ[i].id); //mark it as complete, ready to be removed when it hits the head of the ROB
				// UNCOMMENT FOR DEBUG
				//printf("EX : Type: %d Inst_num: %d\n",schedQ[i].FU,schedQ[i].id);
				//printf("inst %d marked completed\n",schedQ[i].id);
			}
		}
	}
	//print schedQ contents for debug
	/*
	printf("schedQ entries: ");
	for(int j=0;j<(int)schedSize;j++){
	printf("%d/%d/%d/%d ",schedQ[j].id,schedQ[j].FU,schedQ[j].occupied,schedQ[j].marked_to_fire);
	}
	printf("\n%d",numFree);
	printf("\n");
	*/
}

void scheduler_::mark_to_execute(physFile& preg){
	for(int32_t unit=0; unit<3; unit++){ //loop through FU types
		int unitsFilled = 0;
		int notFilled = 0;
		while((unitsFilled < k_avail[unit]) && (notFilled == 0)){ //loop until either all units are filled or all entires are checked without firing
			int lowID = INT_MAX;
			int index_to_ex = -1;
			for(int i=0; i<(int)schedSize; i++){
				if((schedQ[i].completed == 0) && (schedQ[i].FU == unit) && (schedQ[i].occupied == 1) && (schedQ[i].marked_to_fire == 0)){ //check if the entry is occupied, matches the unit, is not completed, and is not currently firing
					if((preg.isReady(schedQ[i].src1_preg) && preg.isReady(schedQ[i].src2_preg))){ //check if PREGs are ready	
						if(schedQ[i].id < lowID){
							lowID = schedQ[i].id; //make sure that the lowest instruction ID fires first
							index_to_ex = i;
						}
					}
				}
			}
			if(index_to_ex > -1){
				schedQ[index_to_ex].marked_to_fire = 1; //mark to fire
				unitsFilled++; //count how many units are filled
				// UNCOMMENT FOR DEBUG
				//printf("SCHED Inst Num: %d \n",schedQ[index_to_ex].id);
			}
			else{
				notFilled = 1;
			}
		}
	}
}

void scheduler_::clear(){
	free(schedQ); //empty scheduling queue
}


state_update SU;
const int numAregs = 32;
int32_t rat[numAregs];
uint64_t numk0, numk1, numk2, numPreg, dispRate, robSize, schedSize;

scheduler_ scheduler;
physFile pregs;
int instNum = 0;
int clock = 0;

void setup_proc(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t rob_, uint64_t preg) 
{
	numk0 = k0; numk1 = k1; numk2 = k2; //store the inputs in global variables
	dispRate = f; robSize = rob_; numPreg = preg;
	schedSize = 2*(k0 + k1 + k2); //calculate scheduler size
	SU.init(robSize); //initialize ROB
	scheduler.init(schedSize,k0,k1,k2); //initialize scheduler
	pregs.init(numPreg); //initialize pregs
	for(int i=0;i<numAregs;i++){
		rat[i]=-1; //set all entries in rat to -1 at the beginning
	}
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
	int num_to_dispatch = (int)dispRate; //set the number of dispatches in this cycle to the minimum of the number of free slots in the rob, the number of free slots in the scheduler, and the dispatch width
	if(SU.numFree < num_to_dispatch){
		num_to_dispatch = SU.numFree;
	}
	if(scheduler.numFree < num_to_dispatch){
		num_to_dispatch = scheduler.numFree;
	}
	int i=0;
	while((i < num_to_dispatch) && (pregs.numFree > 0)){ //make sure there is a free preg
		if(!(notFinalInst = read_instruction(&instruction))){ //read instruction, if there are no instructions left break. record if it is the last instruction
			break;
		}
		i++; //increment how many we have fetched
		instNum++; //increment the current instruction number
		int32_t opcode = instruction.op_code;
		if(opcode == -1){
			opcode = 1; //if the opcode is -1, make it 1
		}
		int32_t src1 = instruction.src_reg[0];
		int32_t src2 = instruction.src_reg[1];
		int32_t dest = instruction.dest_reg;
		int32_t preg1;
		if(src1 > -1){
			preg1 = rat[src1]; //if the src reg exists, index into the rat to find the preg
		}
		else preg1 = -1; //else set it to -1
		int32_t preg2;
		if(src2 > -1){
			preg2 = rat[src2]; //same
		}
		else preg2 = -1;
		int32_t prev;
		if(dest > -1){
			prev = rat[dest]; //same except dest
		}
		else prev = -1;
		int32_t preg = -1;
		if(dest > -1){ //if there is a dest reg
			preg = pregs.fill_first_preg(); //fill a new preg
			rat[dest] = preg; //record the index in the rat
		}
		//create scheduler and rob entries
		newrob = {.occupied = 1, .busy = 1, .areg = dest, .prevPreg = prev, .preg = preg, .id = instNum};
		newsched = {.occupied = 1, .marked_to_fire = 0, .id = instNum, .dest_areg = dest, .dest_preg = preg, .src1_preg = preg1, .src2_preg = preg2, .FU=opcode, .completed = 0};
		//add them to the respective objects
		SU.add_entry(&newrob);
		scheduler.add_entry(&newsched);
	}

}

void run_proc(proc_stats_t* p_stats){
	while((notFinalInst == 1) || (SU.numFree < (int)robSize)){
		clock++;
		// UNCOMMENT FOR DEBUG
		//printf("------------------------------------------  Cycle %d 		 ---------------------------------\n",clock);
		SU.update(pregs,scheduler); //state update
		scheduler.complete_instructions(SU, pregs); //execute
		scheduler.mark_to_execute(pregs); //schedule
		dispatch(); //dispatch
		// UNCOMMENT FOR DEBUG
		//printf("---------------------------------------------------------------------------------------------------\n\n");
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
	//free all pointers
	SU.clear();
	scheduler.clear();
	pregs.clear();
	//record stats
	p_stats -> cycle_count = clock;
	p_stats -> retired_instruction = instNum;
	p_stats -> avg_inst_fired = ((float)instNum)/((float)clock);
	p_stats -> avg_inst_retired = ((float)instNum)/((float)clock);
}