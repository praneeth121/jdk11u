#ifndef RemoteMem_H
#define RemoteMem_H

#include <errno.h>
#include <netdb.h>
// namespace rdma_conn {
// }

#include <rdma/rdma_cma.h>
#include <infiniband/verbs.h>

// #include "gc/shenandoah/shenandoahHeap.hpp"
// #include "memory/virtualspace.hpp"

#define BUFFER_SIZE 16
#define RW_BUFFER_SIZE 1 << 16 // 64KB shoud be adquate, if not, do multiple trips

#define MR_SIZE 1 << 32 // 4GB

#define SERVER_ID_SHIFT 49
#define MR_ID_SHIFT 34
#define MR_OFFSET_SHIFT 2

#define SERVER_ID_BITS 15
#define MR_ID_BITS 15
#define MR_OFFSET_BITS 32

#define REMOTE_OOP_MASK 1 << 1
#define MR_OFFSET_MASK ((1 << MR_OFFSET_BITS) - 1) << MR_OFFSET_SHIFT
#define SERVER_ID_MASK ((1 << SERVER_ID_BITS) - 1) << SERVER_ID_SHIFT
#define MR_ID_MASK ((1 << MR_ID_BITS) - 1) << MR_ID_SHIFT


class RemoteMem;
class RDMAServer;
// class RemoteRegion;
class ShenandoahHeap;

enum DISCONNECT_CODE {
    out_destroy_listen_ep,
    out_free_addrinfo,
    out_destroy_accept_ep,
    out_dereg_recv,
    out_dereg_send,
    out_disconnect,

};

enum COMM_CODE {
    BEGIN,
	REG_MR,
	EXIT,
    TEST = 10
};


// =================UtilFunctions============================= 


static void print_a_buffer(char* buffer, int buffer_size, char* buffer_name) {
	tty->print("%s: ", buffer_name);
	for (int i = 0; i < buffer_size; i++) {
		tty->print("%02X ", buffer[i]);
	}
	tty->print("\n");
}

static void get_wc_status(struct ibv_wc* wc) {
	tty->print_cr("WC Status: %d", wc->status);
	tty->print_cr("WC Opcode: %d", wc->opcode);
	tty->print_cr("--------------");
}

static int rdma_post_read(struct rdma_cm_id *id, void *context, void *addr,
	       size_t length, struct ibv_mr *mr, int flags,
	       uint64_t remote_addr, uint32_t rkey)
{
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr->lkey;


	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.opcode = IBV_WR_RDMA_READ;
	wr.send_flags = flags;
	wr.wr.rdma.remote_addr = remote_addr;
	wr.wr.rdma.rkey = rkey;

	return ibv_post_send(id->qp, &wr, &bad);
}

static int rdma_post_write(struct rdma_cm_id *id, void *context, void *addr,
		size_t length, struct ibv_mr *mr, int flags,
		uint64_t remote_addr, uint32_t rkey)
{
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr ? mr->lkey : 0;

	// return rdma_post_writev(id, context, &sge, 1, flags, remote_addr, rkey);

	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.opcode = IBV_WR_RDMA_WRITE;
	wr.send_flags = flags;
	wr.wr.rdma.remote_addr = remote_addr;
	wr.wr.rdma.rkey = rkey;

	return ibv_post_send(id->qp, &wr, &bad);
}

static int rdma_post_send(struct rdma_cm_id *id, void *context, void *addr,
	       size_t length, struct ibv_mr *mr, int flags)
{
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr ? mr->lkey : 0;

	// return rdma_post_sendv(id, context, &sge, 1, flags);

	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.opcode = IBV_WR_SEND;
	wr.send_flags = flags;

	return ibv_post_send(id->qp, &wr, &bad);
}

static int rdma_post_recv(struct rdma_cm_id *id, void *context, void *addr,
	       size_t length, struct ibv_mr *mr)
{
	struct ibv_sge sge;

	assert((addr >= mr->addr) &&
		(((uint8_t *) addr + length) <= ((uint8_t *) mr->addr + mr->length)), "Fail");
	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) length;
	sge.lkey = mr->lkey;

	// return rdma_post_recvv(id, context, &sge, 1);

	struct ibv_recv_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;

	if (id->srq)
		return ibv_post_srq_recv(id->srq, &wr, &bad);
	else
		return ibv_post_recv(id->qp, &wr, &bad);
}

static int rdma_get_recv_comp(struct rdma_cm_id *id, struct ibv_wc *wc)
{
	struct ibv_cq *cq;
	void *context;
	int ret;

	do {
		ret = ibv_poll_cq(id->recv_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_req_notify_cq(id->recv_cq, 0);
		if (ret)
			return ret;

		ret = ibv_poll_cq(id->recv_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_get_cq_event(id->recv_cq_channel, &cq, &context);
		if (ret)
			return ret;

		assert(cq == id->recv_cq && context == id, "Fail");
		ibv_ack_cq_events(id->recv_cq, 1);
	} while (1);
	return ret;
}

static int rdma_get_send_comp(struct rdma_cm_id *id, struct ibv_wc *wc)
{
	struct ibv_cq *cq;
	void *context;
	int ret;

	do {
		ret = ibv_poll_cq(id->send_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_req_notify_cq(id->send_cq, 0);
		if (ret)
			return ret;

		ret = ibv_poll_cq(id->send_cq, 1, wc);
		if (ret)
			break;

		ret = ibv_get_cq_event(id->send_cq_channel, &cq, &context);
		if (ret)
			return ret;

		assert(cq == id->send_cq && context == id, "Fail");
		ibv_ack_cq_events(id->send_cq, 1);
	} while (1);
	return ret;
}

static int rdma_post_cas(struct rdma_cm_id *id, void *context, void *addr,
		struct ibv_mr *mr, int flags,
		uint64_t remote_addr, uint32_t rkey, uint64_t expected, uint64_t desired)
{
	/*
	A 64 bits value in a remote QP's virtual space is being read, compared with 
	wr.atomic.compare_add and if they are equal, the value wr.atomic.swap is being 
	written to the same memory address, in an atomic way. No Receive Request will 
	be consumed in the remote QP. The original data, before the compare operation, 
	is being written to the local memory buffers specified in sg_list
	*/
	struct ibv_sge sge;

	sge.addr = (uint64_t) (uintptr_t) addr;
	sge.length = (uint32_t) 64; // must be 64 bits
	sge.lkey = mr ? mr->lkey : 0;

	// return rdma_post_writev(id, context, &sge, 1, flags, remote_addr, rkey);

	struct ibv_send_wr wr, *bad;

	wr.wr_id = (uintptr_t) context;
	wr.next = NULL;
	wr.sg_list = &sge;
	wr.num_sge = 1;
	wr.opcode = IBV_WR_ATOMIC_CMP_AND_SWP;
	wr.send_flags = flags;
	wr.wr.atomic.remote_addr = remote_addr;
	wr.wr.atomic.rkey = rkey;
	wr.wr.atomic.compare_add = expected;
	wr.wr.atomic.swap = desired;

	return ibv_post_send(id->qp, &wr, &bad);
}


// class RemoteRegion : public CHeapObj<mtGC> {

// private:
//     RemoteMem* connection;

//     uint32_t rkey;  // rkey of remote memory to be used at local node
//     void* remote_addr;	// addr of remote memory to be used at local node	

//     uint8_t rw_buff[RW_BUFFER_SIZE];

//     struct ibv_mr *rw_mr;
//     int send_flags;
//     struct ibv_wc wc;

	

// public:
//     RemoteRegion(RemoteMem* connection, uint32_t rkey, void* remote_addr);
//     int initialize();
//     int disconnect(DISCONNECT_CODE code);
//     // int rdma_send(uint8_t* buffer, int length);
//     // int rdma_recv(uint8_t* buffer, int length);

//     int rdma_read(uint8_t* buffer, int length, int addr_offset);
//     int rdma_write(uint8_t* buffer, int length, int addr_offset);

// 	int get_new_region();


//     void print_buffers() {
//         print_a_buffer(send_msg, BUFFER_SIZE, (char*)"send_msg");
//         print_a_buffer(recv_msg, BUFFER_SIZE, (char*)"recv_msg");
//         tty->print_cr("-----------");
//     }




//     uint32_t get_rkey () {
//         return rkey;
//     }

//     void* get_remote_addr () {
//         return remote_addr;
//     }

// };


class MemoryRegion : public CHeapObj<mtGC> {
private:
	uint32_t _rkey;
	void* _addr;
	size_t _mr_size;
	// size_t _allocation_offset; // offset from mr->addr where an allocation can happen
	size_t _used; 

public:

private:

public:
	MemoryRegion(uint32_t rkey, void* start_addr, size_t mr_size);
	// size_t allocation_offset() 	{ return _allocation_offset; }
	size_t used() 						{ return _used; }
	size_t free() 						{ return (size_t)(_mr_size) - _used; }
	uint32_t rkey() 					{ return _rkey; }
	void* start_addr() 					{ return _addr; }
	// void* allocation_pointer()			{ return (uint8_t*)_addr + _allocation_offset; }

	void increment_used(int i)			{_used += i;}
};


class RDMAServer : public CHeapObj<mtGC> {
private:
	RemoteMem* connection;
	MemoryRegion* mr;
	// RemoteRegion* rr_arr[100]; //allocate 100 regions each server
	int next_empty_idx;
	int send_flags;
	struct ibv_wc wc;

    char send_msg[BUFFER_SIZE];
    char recv_msg[BUFFER_SIZE];
	char rdma_buff[RW_BUFFER_SIZE];
    struct ibv_mr *recv_mr, *send_mr, *rdma_mr;


	// struct ibv_mr * mr []


public:
	int idx;
	// attributes for communication
	struct rdma_cm_id *cm_id;

private:
	MemoryRegion* get_current_mr(size_t length);



public:
	RDMAServer(int idx, RemoteMem* connection);
	size_t mr_size();
	int send_comm_code(COMM_CODE code, size_t optional_val=0);
	MemoryRegion* reg_new_mr(size_t byte_size);
	int register_comm_mr();
	int register_rdma_mr();
	

	int rdma_write(char* buffer, size_t length, size_t offset);
	int rdma_read(char* buffer, size_t length, size_t addr_offset);

	int rdma_send(char* buff);
	int rdma_recv(char* buff);

	int atomic_cas(uint64_t& previous, size_t offset, uint64_t expected, uint64_t desired);

	int disconnect(DISCONNECT_CODE code);
};



class RemoteMem : public CHeapObj<mtGC> {

/* 

Responsible for control path 

Each mem region is set to be 4GB, each expansion would register extra 4GB at remote nodes

A pointer to a remote location is made up of
[       32 bit rkey     |       32 bit mr offset        ]

allocation is bump-pointing in remote mem region



*/


private:

    static ShenandoahHeap* _heap;
	static RemoteMem* _remote_mem;
    size_t num_connections;
	ReservedSpace _rs;
    struct rdma_cm_id* _listen_id;

    // RemoteRegion** rr_arr;
	RDMAServer** server_arr;

    struct rdma_addrinfo *res;

    // void* current_head;  // Head of allocation
    int current_rr_idx;
    uint32_t current_offset;

public:

    RemoteMem(ShenandoahHeap* heap, size_t num_connections, ReservedSpace rs);

	static RemoteMem* remote_mem();
    int establish_connections	();
    int disconnect				(DISCONNECT_CODE code);
	// TODO: see if this should be deleted
    bool expand_remote_region(int rm_idx, int expansion_size);


	// functions that mimic local mem read and write api
	void zero_to_words(HeapWord* addr, size_t word_size);




	// getters
	char*					base()				const	{ return _rs.base(); }
	char*					end()				const	{ return _rs.end(); }
	size_t					size()				const	{ return _rs.size(); }
	size_t					size_per_server()	const	{ return _rs.size() / num_connections; }
    struct rdma_cm_id ** 	listen_id()			{ return &_listen_id; }
	size_t 				 	mr_size()			{ return size_per_server(); }

	// util funcs
	void 	addr_to_pos	(char* addr, size_t& server_idx, size_t& offset);
	char* 	pos_to_addr	(size_t serer_idx, size_t offset);

	// write
	void	write			(char* to_addr, char* from_buffer, size_t byte_length);
	void	write			(HeapWord* to_addr, HeapWord* from_buffer, size_t word_length);
	template <typename T>
	void	write			(T val, void* to_addr);
	void	write_obj_header(oopDesc header, void* to_addr);

	// reads
	void	read			(char* from_addr, char* to_buffer, size_t byte_length);
	void	read 			(HeapWord* from_addr, HeapWord* to_buffer, size_t word_length);
	template <typename T>
	T		read			(void* from_addr);
	oopDesc	read_obj_header	(void* from_addr);

	// atomic cas
	uint64_t remote_cas		(char* to_addr, uint64_t expected, uint64_t desired);

	bool is_in(const void* p) const {
		return p >= (void*)base() && p < (void*)end();
	}

	// testing funcs
    void perform_some_tests();

private:

	// int send_msg(int node_idx, uint8_t* msg);
	// int recv_msg(uint8_t* msg);






    // uint32_t get_current_offset() {
    //     return current_offset;
    // }

    // uint32_t get_current_rkey() {
    //     return rr_arr[current_rr_idx]->get_rkey();
    // }

    // uint64_t get_current_addr() {
    //     return (uint64_t)get_current_rkey() << 32 | (uint64_t)get_current_offset();
    // }
};

#endif