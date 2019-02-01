#pragma once

#include <linux/types.h>

#define KVI_EVENT_CR          (1 << 1)	/* control register was modified */
#define KVI_EVENT_MSR         (1 << 2)	/* model specific reg. was modified */
#define KVI_EVENT_XSETBV      (1 << 3)	/* ext. control register was modified */
#define KVI_EVENT_BREAKPOINT  (1 << 4)	/* breakpoint was reached */
#define KVI_EVENT_HYPERCALL   (1 << 5)	/* user hypercall */
#define KVI_EVENT_PAGE_FAULT  (1 << 6)	/* hyp. page fault was encountered */
#define KVI_EVENT_TRAP        (1 << 7)	/* trap was injected */
#define KVI_EVENT_DESCRIPTOR  (1 << 8)	/* descriptor table access */
#define KVI_EVENT_CREATE_VCPU (1 << 9)
#define KVI_EVENT_PAUSE_VCPU  (1 << 10)

#define KVI_PAGE_ACCESS_R (1 << 0)
#define KVI_PAGE_ACCESS_W (1 << 1)
#define KVI_PAGE_ACCESS_X (1 << 2)

#define KVI_DESC_IDTR	1
#define KVI_DESC_GDTR	2
#define KVI_DESC_LDTR	3
#define KVI_DESC_TR		4

#define KVI_CS_TYPE_16_BIT			1
#define KVI_CS_TYPE_32_BIT			2
#define KVI_CS_TYPE_64_BIT			3

#define KVI_CPL_KERNEL						0
#define KVI_CPL_USER						3

struct hvi_event_cr {
	__u16 cr;
	__u16 padding[3];
	unsigned long old_value;
	unsigned long new_value;
};

struct hvi_event_breakpoint {
	__u64 gpa;
};

struct hvi_event_page_fault {
	__u64 gva;
	__u64 gpa;
	__u32 mode;
	__u32 padding;
};


struct hvi_event_msr {
	__u32 msr;
	__u32 padding;
	__u64 old_value;
	__u64 new_value;
};

struct hvi_event_trap {
	__u32 vector;
	__u32 type;
	__u32 error_code;
	__u32 padding;
	__u64 cr2;
};

struct hvi_event_descriptor {
	union {
		struct {
			__u32 instr_info;
			__u32 padding;
			__u64 exit_qualification;
		} vmx;
		struct {
			__u64 exit_info;
			__u64 padding;
		} svm;
	} arch;
	__u8 descriptor;
	__u8 write;
	__u8 padding[6];
};

typedef struct hvi_x86_gpr
{
	uint64_t rax;	//r0
	uint64_t rcx;	//r1
	uint64_t rdx;	//r2
	uint64_t rbx;	//r3
	uint64_t rsp;	//r4
	uint64_t rbp;	//r5
	uint64_t rsi;	//r6
	uint64_t rdi;	//r7
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
}hvi_x86_gpr_t;

struct x86_segment {
	__u64 base;
	__u32 limit;
	__u16 selector;
	__u16 padding;
	__u32 ar;
};

struct x86_dtable {
	__u64 base;
	__u16 limit;
	__u16 padding[3];
};

/*  */
struct x86_sregs {
	struct x86_segment cs, ds, es, fs, gs, ss;	
};

typedef struct x86_regs {
	hvi_x86_gpr_t gprs;
	uint64_t rflags;
	uint64_t dr7;
	uint64_t rip;
	uint64_t cr0;
	uint64_t cr3;
	uint64_t cr4;
	uint64_t rsp;
	uint64_t sysenter_cs;
	uint64_t sysenter_esp;
	uint64_t sysenter_eip;
	uint64_t msr_efer;
	uint64_t fs_base;
	uint64_t gs_base;
	uint32_t cs_arbytes;
	uint32_t _pad;
} hvi_x86_registers_t;

typedef enum
{
	registers_state,
	msr,
	idtr,
	gdtr,
	cpu_count,
	current_tid,
	gpr_registers_state,
	cs_type,
	cs_ring,
	seg_registers_state	
}hvi_query_info_e;

typedef enum
{
	ept_violation = 1,
	msr_write = 2,
	cr_write,
	xsetbv_modification,
	xcr_modification,
	breakpoint,
	vmcall,
	mtf_exit
}hv_event_e;

/*
 *Prototype for event report callback function.
 **/
typedef int (*hv_event_callback)(hv_event_e type, unsigned char* data, int size);

/*
 *Query specific guest information.
 **/
int hvi_query_guest_info(int vcpu, hvi_query_info_e query_type, unsigned char* param, unsigned char* buffer, int* size);

/*
 *Set rflags register of specified vcpu.
 **/
int hvi_set_register_rflags(int vcpu, unsigned long new_value);

/*
 *Set rip register of specified vcpu.
 **/
int hvi_set_register_rip(int vcpu, unsigned long new_value);
	
/*
 *Pause all vcpus.
 **/
int hvi_request_vcpu_pause(void);

/*
 *Resume paused vcpus.
 **/
int hvi_request_vcpu_resume(void);

/*
 *Map a guest physical adress inside the hvi address space.
 **/
int hvi_physmem_map_to_host(unsigned long phy_addr, unsigned long length, unsigned long flags, void** host_ptr);

/*
 *Unmap a page which is previously mapped.
 **/
int hvi_physmem_unmap(void** host_ptr);

/*
 *Give GPA address, query the EPT access rights.
 **/
int hvi_get_ept_page_protection(unsigned long addr, unsigned char *read, unsigned char *write, unsigned char* execute);

/*
 *Modify the EPT access rights for the indicated GPA address.
 **/
int hvi_set_ept_page_protection(unsigned long addr, unsigned char read, unsigned char write, unsigned char execute);

/*
 *Modify whether write msr causes vmexit.
 **/
int hvi_modify_msr_write_exit(unsigned long msr, unsigned char is_enable);

/*
 *Modify whether write cr causes vmexit.
 **/
int hvi_modify_cr_write_exit(unsigned long cr, unsigned int mask, unsigned char is_enable);

/*
 *Inject a #PF in guest.
 **/
int hvi_force_guest_page_fault(unsigned long virtual_addr, unsigned long error);

/*
 *Enable mtf.*/
int hvi_enable_mtf(void);

/*
 *Disable mtf.
 **/
int hvi_disable_mtf(void);

/*
 *Register event report call back
 **/
int hvi_register_event_callback(hv_event_callback hvi_event_handler);

/*
 *Un-register event report call back
 **/
int hvi_unregister_event_callback(void);
