#ifndef PROCSIM_HPP
#define PROCSIM_HPP

#include <cstdint>
#include <cstdio>
#include <cinttypes>
#include <cstdlib>

#define DEFAULT_K0 3
#define DEFAULT_K1 2
#define DEFAULT_K2 1
#define DEFAULT_ROB 12
#define DEFAULT_F 4
#define DEFAULT_PREG 32

typedef struct _proc_inst_t
{
    uint32_t instruction_address;
    int32_t op_code;
    int32_t src_reg[2];
    int32_t dest_reg;
    
    // You may introduce other fields as needed
    
} proc_inst_t;

typedef struct _proc_stats_t
{
    float avg_inst_retired;
    float avg_inst_fired;
    unsigned long retired_instruction;
    unsigned long cycle_count;
    
} proc_stats_t;

bool read_instruction(proc_inst_t* p_inst);

void setup_proc(uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t rob, uint64_t preg);
void run_proc(proc_stats_t* p_stats);
void complete_proc(proc_stats_t* p_stats);

typedef struct _preg{
	int ready;
	int occupied;
} pregEntry;

typedef struct _robEntry{
	int occupied;
	int busy;
	int32_t areg;
	int32_t prevPreg;
	int32_t preg;
	int id;
} robEntry;

typedef struct _schedEntry{
	int occupied;
	int32_t marked_to_fire;
	int id;
	int32_t dest_areg;
	int32_t dest_preg;
	int32_t src1_preg;
	int32_t src2_preg;
	int32_t FU;
	int completed;
} schedEntry;

//typedef struct _function_unit{
//	int32_t free;
//	uint32_t instruction_address;
//} function_unit;
class state_update;
class physFile;

class scheduler_ {
public:
	scheduler_();
	void init(uint64_t size, uint64_t k0, uint64_t k1, uint64_t k2);
	bool add_entry(schedEntry * newEntry);
	void remove(int id);
	void sort_by_id();
	void complete_instructions(state_update& SU, physFile& preg);
	void mark_to_execute(physFile& preg);
	void clear();
	int numFree;
	uint64_t schedSize;
	schedEntry* schedQ;
	int k_avail[3];
};

#endif /* PROCSIM_HPP */
