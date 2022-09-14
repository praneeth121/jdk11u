// #include "memory/remoteMem.hpp"
// #include "gc/shenandoah/shenandoahHeap.hpp"
#include "gc/shenandoah/shenandoahHeapRegion.inline.hpp"
#include "memory/resourceArea.inline.hpp"

ShenandoahHeap* RemoteMem::_heap = NULL;
RemoteMem* RemoteMem::_remote_mem = NULL;

// ==============RemoteRegion=======================================

// RemoteRegion::RemoteRegion(RemoteMem* connection) : 
// 	connection(connection),
// 	rkey(0),
// 	remote_addr(NULL),
// 	send_msg(),
// 	recv_msg(),
// 	rw_buff(),
// 	id(NULL),
// 	mr(NULL),
// 	send_mr(NULL),
// 	rw_mr(NULL),
// 	send_flags(0),
// 	wc() {
	
// 	initialize();
// }


// int RemoteRegion::initialize () {
//     // struct rdma_addrinfo hints;
// 	// struct ibv_qp_init_attr init_attr;
// 	// struct ibv_qp_attr qp_attr;
// 	// int ret;
//     // void* temp;


//     // tty->print_cr("Getting connection request ...");
// 	// ret = rdma_get_request(*connection->listen_id(), &id);
// 	// // ret = rdma_get_request(NULL, NULL);
// 	// if (ret) {
// 	// 	perror("rdma_get_request");
//     //     return connection->disconnect(out_destroy_listen_ep);
// 	// 	// goto out_destroy_listen_ep;
// 	// }


// 	// memset(&qp_attr, 0, sizeof qp_attr);
// 	// memset(&init_attr, 0, sizeof init_attr);
// 	// ret = ibv_query_qp(id->qp, &qp_attr, IBV_QP_CAP,
// 	// 		   &init_attr);
// 	// if (ret) {
// 	// 	perror("ibv_query_qp");
//     //     return disconnect(out_destroy_accept_ep);
// 	// 	// goto out_destroy_accept_ep;
// 	// }
// 	// if (init_attr.cap.max_inline_data >= 16)
// 	// 	send_flags = IBV_SEND_INLINE;
// 	// else
// 	// 	tty->print_cr("rdma_server: device doesn't support IBV_SEND_INLINE, "
// 	// 	       "using sge sends");

//     // tty->print_cr("Registering mr ...");
// 	// // mr = rdma_reg_msgs(id, recv_msg, 16);
// 	// mr = ibv_reg_mr(id->pd, recv_msg, 16, IBV_ACCESS_LOCAL_WRITE);
// 	// if (!mr) {
// 	// 	ret = -1;
// 	// 	perror("rdma_reg_msgs for recv_msg");
//     //     return disconnect(out_destroy_accept_ep);
// 	// 	// goto out_destroy_accept_ep;
// 	// }
// 	// if ((send_flags & IBV_SEND_INLINE) == 0) {
//     //     tty->print_cr("Registering send mr ...");
// 	// 	// send_mr = rdma_reg_msgs(id, send_msg, 16);
// 	// 	send_mr = ibv_reg_mr(id->pd, send_msg, 16, IBV_ACCESS_LOCAL_WRITE);
// 	// 	if (!send_mr) {
// 	// 		ret = -1;
// 	// 		perror("rdma_reg_msgs for send_msg");
//     //         return disconnect(out_dereg_recv);
// 	// 		// goto out_dereg_recv;
// 	// 	}
// 	// }

//     // // tty->print_cr("Server accepting connection ...");
// 	// ret = rdma_accept(id, NULL);
// 	// if (ret) {
// 	// 	perror("rdma_accept");
//     //     disconnect(out_dereg_send);
// 	// 	// goto out_dereg_send;
// 	// }

//     // receive remote mr rkey and addr of client


// 	// tty->print_cr("rdma_post_recv ...");
// 	// ret = rdma_post_recv(id, NULL, recv_msg, 16, mr);
// 	// if (ret) {
// 	// 	perror("rdma_post_recv");
//     //     disconnect(out_dereg_send);
// 	// 	// goto out_dereg_send;
// 	// }


// 	// tty->print_cr("rdma_get_recv_comp ...");
// 	// while ((ret = rdma_get_recv_comp(id, &wc)) == 0);
// 	// if (ret < 0) {
// 	// 	perror("rdma_get_recv_comp");
//     //     return disconnect(out_disconnect);
// 	// 	// goto out_disconnect;
// 	// }

// 	uint8_t temp_buff[BUFFER_SIZE];
// 	rdma_recv(temp_buff, BUFFER_SIZE);

// 	print_a_buffer(temp_buff, 16, "temp_buff");

// 	// // extract information
// 	tty->print_cr("rkey: %d", rkey);

// 	memcpy(&rkey, temp_buff, sizeof(rkey));
// 	tty->print_cr("rkey: %d", rkey);
// 	print_a_buffer((uint8_t*)&rkey, sizeof(rkey), (char*)"rkey");

// 	memcpy(&remote_addr, temp_buff + sizeof(rkey), sizeof(remote_addr));

// 	print_a_buffer((uint8_t*)&remote_addr, sizeof(remote_addr), (char*)"remote addr");

//     // register a local buffer
// 	// register remote region for rdma read and write
// 	rw_mr = ibv_reg_mr(id->pd, rw_buff, RW_BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE);
// 	strcpy((char*)rw_buff, "Dat");
	
// 	print_a_buffer(rw_buff, BUFFER_SIZE, (char*)"rw_buff");

// 	ret = rdma_post_write(id, NULL, rw_buff, BUFFER_SIZE, rw_mr, IBV_SEND_SIGNALED, (uint64_t)remote_addr, rkey);
// 	if (ret) {
//         perror("Error rdma write");
//         return disconnect(out_disconnect);
//     }
// 	do {
//         ret = ibv_poll_cq(id->send_cq, 1, &wc);
//     } while (ret == 0);

// 	memset(rw_buff, 0, RW_BUFFER_SIZE);
// 	get_wc_status(&wc);


//     return ret;

// }

// int RemoteRegion::disconnect(DISCONNECT_CODE code) {
//     switch (code){
//         // case out_dereg_recv:
//         //     ibv_dereg_mr(recv_mr);
//         case out_destroy_accept_ep:
//             rdma_destroy_ep(id);
//         case out_destroy_listen_ep:
//             rdma_destroy_ep(*connection->listen_id());
//         case out_free_addrinfo:
//             connection->disconnect(out_free_addrinfo);

//         default:
//             return 0;
//     }
// }

int RDMAServer::rdma_read(char* buffer, size_t length, size_t offset) {
	int ret;
	
	tty->print_cr("RDMA read ...");

	memset(comm_buff(), 0, RW_BUFFER_SIZE);

	ret = rdma_post_read(cm_id, NULL, comm_buff(), length, comm_mr, IBV_SEND_SIGNALED, (uint64_t)((char*)(mr->start_addr())+offset), mr->rkey());
	if (ret) {
		perror("Error rdma read");
		return disconnect(out_disconnect);
	}
	do {
		ret = ibv_poll_cq(cm_id->send_cq, 1, &wc);
	} while (ret == 0);

	get_wc_status(&wc);

	memcpy(buffer, comm_buff(), length);

	return ret;	
}

int RDMAServer::rdma_write(char* buffer, size_t length, size_t offset) {
	assert(false, "not writing anything yet");
	int ret;
	
	// MemoryRegion* mr = get_current_mr(length);
	assert(length < mr->free(), "No valid free space");
	tty->print_cr("RDMA write ...");

	memset(comm_buff(), 0, RW_BUFFER_SIZE);
	
	memcpy(comm_buff(), buffer, length);

	ret = rdma_post_write(cm_id, NULL, comm_buff(), length, comm_mr, IBV_SEND_SIGNALED, (uint64_t)((char*)mr->start_addr()) + offset, mr->rkey());
	if (ret) {
		perror("Error rdma write");
		return disconnect(out_disconnect);
		// goto out_disconnect;
	}
	do {
		ret = ibv_poll_cq(cm_id->send_cq, 1, &wc);
	} while (ret == 0);

	// get_wc_status(&wc);

	return ret;	
}

int RDMAServer::atomic_cas(uint64_t& previous, size_t offset, uint64_t expected, uint64_t desired) {
	int ret;
	memset(comm_buff(), 0, RW_BUFFER_SIZE);
	
	ret = rdma_post_cas(cm_id, NULL, comm_buff(), comm_mr, IBV_SEND_SIGNALED, (uint64_t)((char*)mr->start_addr()) + offset, mr->rkey(), expected, desired);
	if (ret) {
		perror("Error atomic cas");
		return disconnect(out_disconnect);
		// goto out_disconnect;
	}
	do {
		ret = ibv_poll_cq(cm_id->send_cq, 1, &wc);
	} while (ret == 0);
	
	memcpy(&previous, comm_buff(), sizeof(uint64_t));

	return ret;
}

// // int RemoteRegion::rdma_send(uint8_t* buffer, int length) {
// // 	int ret;

// // 	tty->print_cr("RDMA send ...");

// // 	memset(send_msg, 0, BUFFER_SIZE);

// // 	memcpy(send_msg, buffer, length);

// // 	ret = rdma_post_send(id, NULL, send_msg, 16, send_mr, send_flags);
// // 	if (ret) {
// // 		perror("rdma_post_send");
// // 		return disconnect(out_disconnect);
// // 		// goto out_disconnect;
// // 	}

// // 	while ((ret = rdma_get_send_comp(id, &wc)) == 0);
// // 	if (ret < 0) {
// // 		perror("rdma_get_send_comp");
// // 		return disconnect(out_disconnect);
// // 		// goto out_disconnect;
// // 	}

// // 	return ret;
// // }

// // int RemoteRegion::rdma_recv(uint8_t* buffer, int length) {
// // 	int ret;
// // 	tty->print_cr("rdma_post_recv ...");

// // 	memset(recv_msg, 0, BUFFER_SIZE);

// // 	ret = rdma_post_recv(id, NULL, recv_msg, BUFFER_SIZE, mr);
// // 	if (ret) {
// // 		perror("rdma_post_recv");
// //         return disconnect(out_dereg_send);
// // 		// goto out_dereg_send;
// // 	}


// // 	tty->print_cr("rdma_get_recv_comp ...");
// // 	while ((ret = rdma_get_recv_comp(id, &wc)) == 0);
// // 	if (ret < 0) {
// // 		perror("rdma_get_recv_comp");
// //         return disconnect(out_disconnect);
// // 		// goto out_disconnect;
// // 	}

// // 	memcpy(buffer, recv_msg, length);

// // 	return ret;
// // }

// =================MemoryRegion=============================
MemoryRegion::MemoryRegion(uint32_t rkey, void* start_addr, size_t mr_size) : 
	_rkey(rkey),
	_addr(start_addr),
	_mr_size(mr_size),
	// _allocation_offset(0),
	_used(0)
{}


// =================RDMAServer=============================
RDMAServer:: RDMAServer(int idx, RemoteMem* remote_mem):
	idx(idx),
	_remote_mem(remote_mem),
	mr(NULL),
	next_empty_idx(0),
	send_flags(0),
	wc(),
	send_msg(NEW_C_HEAP_ARRAY(char, BUFFER_SIZE, mtGC)),
	recv_msg(NEW_C_HEAP_ARRAY(char, BUFFER_SIZE, mtGC)),
	recv_mr(NULL),
	send_mr(NULL),
	rdma_mr(NULL),
	cm_id()
{
	// tty->print_cr("Number of regions per server %lu", ((size_t)1<<expectedMemPerServer) / MR_SIZE);
	// mr = (MemoryRegion**)malloc(sizeof(MemoryRegion*) * ((size_t)1<<expectedMemPerServer) / MR_SIZE);
	// mr = new MemoryRegion()
	struct rdma_addrinfo hints;
	struct ibv_qp_init_attr init_attr;
	struct ibv_qp_attr qp_attr;
	int ret;

	tty->print_cr("Getting connection request ...");
	ret = rdma_get_request(*_remote_mem->listen_id(), &cm_id);
	// ret = rdma_get_request(NULL, NULL);
	if (ret) {
		perror("rdma_get_request");
        _remote_mem->disconnect(out_destroy_listen_ep);
		// goto out_destroy_listen_ep;
	}


	memset(&qp_attr, 0, sizeof qp_attr);
	memset(&init_attr, 0, sizeof init_attr);
	ret = ibv_query_qp(cm_id->qp, &qp_attr, IBV_QP_CAP, &init_attr);
	if (ret) {
		perror("ibv_query_qp");
        disconnect(out_destroy_accept_ep);
		// goto out_destroy_accept_ep;
	}
	if (init_attr.cap.max_inline_data >= 16)
		send_flags = IBV_SEND_INLINE;
	else
		tty->print_cr("rdma_server: device doesn't support IBV_SEND_INLINE, "
		       "using sge sends");


	tty->print_cr("Server accepting connection ...");
	ret = rdma_accept(cm_id, NULL);
	if (ret) {
		perror("rdma_accept");
		disconnect(out_dereg_send);
	}
	
	register_comm_mr();		// to send recv control signals
	register_rdma_mr();		// Buffer and region setup for local to read/write to remote regions
	mr = reg_new_mr(_remote_mem->mr_size());
}

int RDMAServer::send_comm_code(COMM_CODE code, size_t optional_val) {
	int ret;

	tty->print_cr("rdma sending a communication code ...");
    memset(send_msg, 0, BUFFER_SIZE);
    memcpy(send_msg , &code, sizeof(COMM_CODE));

	if (code == REG_MR) {
		// optional val should contain a byte size of the remote mr
		memcpy(send_msg + sizeof(COMM_CODE), &optional_val, sizeof(size_t));
	}

    print_a_buffer(send_msg, BUFFER_SIZE, (char*)"send_msg");
	ret = rdma_post_send(cm_id, NULL, send_msg, 16, send_mr, send_flags);
	if (ret) {
		perror("rdma_post_send");
		disconnect(out_disconnect);
		// out_func(out_disconnect);
	}

    tty->print_cr("rdma_get_send_comp ...");
	while ((ret = rdma_get_send_comp(cm_id, &wc)) == 0);
	if (ret < 0)
		perror("rdma_get_send_comp");
	else
		ret = 0;

    return ret;
}

int RDMAServer::disconnect(DISCONNECT_CODE code) {
	switch (code){
        case out_disconnect:
            rdma_disconnect(cm_id);
        case out_destroy_listen_ep:
        case out_free_addrinfo:
			_remote_mem->disconnect(code);
        case out_dereg_send:
            if ((send_flags & IBV_SEND_INLINE) == 0)
                ibv_dereg_mr(send_mr);
            break;
            
        default:
            return 0;
    }
    return 0;
}

MemoryRegion* RDMAServer::reg_new_mr(size_t byte_size) {
	// Send a signal to remote, commanding it to allocate a new rdma mr and send back the key and starting addr of the mr
	tty->print_cr("Registering new mr of size %lu at remote .. ", byte_size);
    int ret;
	uint32_t rkey;
    void* remote_addr;
    send_comm_code(REG_MR, byte_size);
    	// receive remote mr rkey and addr of client
	tty->print_cr("rdma_post_recv ...");
	ret = rdma_post_recv(cm_id, NULL, recv_msg, 16, recv_mr);
	if (ret) {
		perror("rdma_post_recv");
		_remote_mem->disconnect(out_dereg_send);
	}


	tty->print_cr("rdma_get_recv_comp ...");
	while ((ret = rdma_get_recv_comp(cm_id, &wc)) == 0);
	if (ret < 0) {
		perror("rdma_get_recv_comp");
		_remote_mem->disconnect(out_disconnect);
	}

	// extract information

	// todo: if is recv buffer zero then remote region cannot register new MR

	memcpy(&rkey, recv_msg, sizeof(rkey));
	print_a_buffer((char*)&rkey, sizeof(rkey), (char*)"rkey");

	memcpy(&remote_addr, recv_msg + sizeof(rkey), sizeof(remote_addr));

	print_a_buffer((char*)&remote_addr, sizeof(remote_addr), (char*)"remote addr");

	// mr_arr[next_empty_idx] = new MemoryRegion(rkey, remote_addr);
	// next_empty_idx++;
    // return ret;

	return new MemoryRegion(rkey, remote_addr, byte_size);
}

int RDMAServer::register_comm_mr() {
	int ret;

    tty->print_cr("Registering mr ...");
	// mr = rdma_reg_msgs(id, recv_msg, 16);
	recv_mr = ibv_reg_mr(cm_id->pd, recv_msg, 16, IBV_ACCESS_LOCAL_WRITE);
	if (!recv_mr) {
		ret = -1;
		perror("rdma_reg_msgs for recv_msg");
        return disconnect(out_destroy_accept_ep);
		// goto out_destroy_accept_ep;
	}
	if ((send_flags & IBV_SEND_INLINE) == 0) {
        tty->print_cr("Registering send mr ...");
		// send_mr = rdma_reg_msgs(id, send_msg, 16);
		send_mr = ibv_reg_mr(cm_id->pd, send_msg, 16, IBV_ACCESS_LOCAL_WRITE);
		if (!send_mr) {
			ret = -1;
			perror("rdma_reg_msgs for send_msg");
            return disconnect(out_dereg_recv);
			// goto out_dereg_recv;
		}
	}

	return ret;
}

int RDMAServer::register_rdma_mr() {
	rdma_mr = ibv_reg_mr(cm_id->pd, _remote_mem->rdma_buff(), ShenandoahHeapRegion::region_size_bytes(), IBV_ACCESS_LOCAL_WRITE);
	comm_mr = ibv_reg_mr(cm_id->pd, _remote_mem->comm_buff(), ShenandoahHeapRegion::region_size_bytes(), IBV_ACCESS_LOCAL_WRITE);
	return 0;
}

size_t RDMAServer::mr_size() {
	return _remote_mem->mr_size();
}

MemoryRegion* RDMAServer::get_current_mr(size_t length) {
	// if (next_empty_idx == 0) {
	// 	reg_new_mr();
	// }
	// MemoryRegion* mr = mr_arr[next_empty_idx-1];
	// tty->print_cr("Selected mr: %p", mr);
	// while (mr->free() < length) {
	// 	tty->print_cr("next_empty_idx: %d", next_empty_idx);
	// 	tty->print_cr("mr->free(): %lu", mr->free());
	// 	tty->print_cr("length: %lu", length);

	// 	reg_new_mr();
	// 	mr = mr_arr[next_empty_idx-1];
	// }
	// tty->print_cr("Returning new MR");

	// return mr;

	return mr;
	
}

char* RDMAServer::rdma_buff() {
	return _remote_mem->rdma_buff();
}

char* RDMAServer::comm_buff() {
	return _remote_mem->comm_buff();
}


// =================RDMAConnect============================= 


RemoteMem::RemoteMem(ShenandoahHeap* heap, size_t num_connections, ReservedSpace rs) :
	num_connections(num_connections),
	_rs(rs),
	_rdma_buff(NULL),
	_comm_buff(NULL),
	_listen_id(NULL),
	server_arr(),
	res(NULL),
	_current_buffer_offset_in_words(0),
	_evacuation_set(NEW_C_HEAP_ARRAY(jbyte, heap->num_regions(), mtGC))
{
	assert(UseShenandoahGC, "Only Shenandoah can use this module");
	_heap = heap;
	_remote_mem = this;
	// _rdma_buff = (char*)malloc(ShenandoahHeapRegion::region_size_bytes());
	// _comm_buff = (char*)malloc(ShenandoahHeapRegion::region_size_bytes());

	_rdma_buff = NEW_C_HEAP_ARRAY(char, ShenandoahHeapRegion::region_size_bytes(), mtGC);
	_comm_buff = NEW_C_HEAP_ARRAY(char, ShenandoahHeapRegion::region_size_bytes(), mtGC);
	memset(_rdma_buff, 0, ShenandoahHeapRegion::region_size_bytes());
	memset(_comm_buff, 0, ShenandoahHeapRegion::region_size_bytes());
	establish_connections();
}

RemoteMem* RemoteMem::remote_mem() {
	assert(_remote_mem != NULL, "Remote Mem is not established");
	return _remote_mem;
}

ShenandoahHeap* RemoteMem::heap() {
	assert(_heap != NULL, "Heap must be established");
	return _heap;
}

int RemoteMem::establish_connections(){
    struct rdma_addrinfo hints;
	struct ibv_qp_init_attr init_attr;
    int ret;
    memset(&hints, 0, sizeof hints);
	hints.ai_flags = RAI_PASSIVE;
	hints.ai_port_space = RDMA_PS_TCP;
	ret = rdma_getaddrinfo(RDMALocalAddr, RDMAPort, &hints, &res);
	if (ret) {
		tty->print_cr("rdma_getaddrinfo: %s", gai_strerror(ret));
		// tty->print_cr("Fail at rdma_getaddrinfo");
		return ret;
	}

    memset(&init_attr, 0, sizeof init_attr);
	init_attr.cap.max_send_wr = init_attr.cap.max_recv_wr = 16;
	init_attr.cap.max_send_sge = init_attr.cap.max_recv_sge = 16;
	init_attr.cap.max_inline_data = 16;
	init_attr.sq_sig_all = 1;
	ret = rdma_create_ep(&_listen_id, res, NULL, &init_attr);
	if (ret) {
		perror("rdma_create_ep");
        return disconnect(out_free_addrinfo);
	}

    tty->print_cr("Server listening ...");
	ret = rdma_listen(_listen_id, 100);
	if (ret) {
		perror("rdma_listen");
		return disconnect(out_destroy_listen_ep);
	}

    // rr_arr = (RemoteRegion**)malloc(sizeof(RemoteRegion*) * num_connections);
	// rr_arr = new RemoteRegion* [num_connections];
	// server_arr = (RDMAServer**)malloc(sizeof(RDMAServer*) * num_connections);
	server_arr = NEW_C_HEAP_ARRAY(RDMAServer*, num_connections, mtGC);

	memset(server_arr, 0, sizeof(RDMAServer*) * num_connections);

    for (size_t i= 0; i < num_connections; i++) {
		tty->print_cr("i = %lu, num_conn = %lu", i, num_connections);

        server_arr[i] = new RDMAServer(i, this);
        // rr_arr[i] = new RemoteRegion(this);
    }
	// perform_some_tests();

    return ret;
}


int RemoteMem::disconnect (DISCONNECT_CODE code) {
    switch (code){
        case out_disconnect:
        case out_dereg_send:
			for (size_t i = 0; i < num_connections; i++) {
				server_arr[i]->disconnect(code);
			}
        case out_destroy_listen_ep:
            rdma_destroy_ep(*listen_id());
        case out_free_addrinfo:
            rdma_freeaddrinfo(res);
        default:
            return 0;
    }
    return 0;
}

void RemoteMem::copy_to_temp_local_region(oop p, HeapWord* addr, size_t size){
	ShenandoahHeapRegion* remote_r = _heap->heap_region_containing(addr);
	assert(remote_r->is_remote(), "Must be a remote region");
	// calculate offset to _bottom of remote region
	// size_t offset_in_words = pointer_delta(remote_r->bottom(), addr);
	// // calculate addr in temp_local
	// ShenandoahHeapRegion* temp_local = remote_r->get_temp_forwarding();
	// assert(temp_local != NULL, "temp local must have been allocated");

	HeapWord* temp_local_addr = remote_r->get_temp_forwarding(addr);
	Copy::aligned_disjoint_words((HeapWord*) p, temp_local_addr, size);
}

void RemoteMem::copy_to_rdma_buffer(oop p, HeapWord* addr, size_t size) {
	// tty->print_cr("Copying to rdme buffer, org: %p", addr);
	// calculate offset into the buffer from recorded coordinations
	// ShenandoahHeapRegion* rr = _heap->heap_region_containing(addr);
	// assert(rr->is_remote(), "Must be a remote region");
	// size_t offset_from_evac_bottom = pointer_delta(rr->evac_bottom(), (HeapWord*)addr);
	// size_t region_evac_offset_into_buffer = rr->rdma_buffer_offset_in_words();

	// size_t object_offset = region_evac_offset_into_buffer + offset_from_evac_bottom;
	HeapWord* to_addr = (HeapWord*)get_corresponding_evac_buffer_address(addr);
	memcpy((void*) to_addr, (void*)p, size * HeapWordSize);
	// tty->print_cr("Successfully copy to rdma buffer: obj1 %p, klass1 %p | obj2 %p, klass2 %p | remote %p", 
	// 		(void*)p, (void*)p->klass_or_null_local(),
	// 		(void*)to_addr, (void*)oop(to_addr)->klass_or_null_local(),
	// 		(void*)addr);
}

void RemoteMem::flush_evac_buff() {
	tty->print_cr("Flushing to remote mem with rdma");
	// assert(false, "Hold it right there");
	// ShenandoahCollectionSet* cset = _heap->collection_set();
	// make a destination set which added regions at evec phase
	// iterate through the destination set and move objects from buffer to its evac bottom
	// be careful about sizing

}

void RemoteMem::fill_with_dummy_object(HeapWord* start, HeapWord* end, bool zap) {
	// tty->print_cr("Filling tlabs with dummy objects");
	assert(start, "Start must not be null");
	assert(end, "End must not be null");
	ShenandoahHeapRegion* rr = _heap->heap_region_containing(start);
	// assert (rr == _heap->heap_region_containing(end-1), "tlab must not cross region bound");
	assert (rr->is_remote(), "Must be operated on remote region");


	// tty->print_cr("start %p", start);
	// tty->print_cr("end %p", end);
	// tty->print_cr("start %p r %lu", start, _heap->heap_region_index_containing(start));
	// tty->print_cr("end %p r %lu", end, _heap->heap_region_index_containing(end));

	HeapWord* buff_start	= (HeapWord*)get_corresponding_evac_buffer_address(start);
	// HeapWord* buff_end		= (HeapWord*)get_corresponding_evac_buffer_address(end);

	// _heap->fill_with_dummy_object(buff_start, buff_end, zap);
	_heap->fill_with_dummy_object(buff_start, pointer_delta(end, start), zap);
}


void RemoteMem::zero_to_words(HeapWord* addr, size_t word_size) {
	assert(_heap->is_in_remote(addr), "Address must be in remote");
	tty->print_cr("RemoteMem::zero_to_words");
	char buff[word_size*HeapWordSize];
	memset(buff, 0, word_size*HeapWordSize);
	write(addr, (HeapWord*)buff, word_size);
}

// int RemoteMem::send_comm_code(COMM_CODE code) {

//     int ret;
//     // now we can close both process by posting a send

// 	tty->print_cr("rdma_post_send ...");
//     memset(send_msg, 0, BUFFER_SIZE);
//     memcpy(send_msg, &code, sizeof(COMM_CODE));
//     print_a_buffer(send_msg, BUFFER_SIZE, "send_msg");
// 	ret = rdma_post_send(id, NULL, send_msg, 16, send_mr, send_flags);
// 	if (ret) {
// 		perror("rdma_post_send");
// 		out_func(out_disconnect);
// 	}

//     tty->print_cr("rdma_get_send_comp ...");
// 	while ((ret = rdma_get_send_comp(id, &wc)) == 0);
// 	if (ret < 0)
// 		perror("rdma_get_send_comp");
// 	else
// 		ret = 0;

//     return ret;
// }

void RemoteMem::perform_some_tests() {
	// RemoteRegion* rr = rr_arr[0];
	RDMAServer* server;
	tty->print_cr("Testing rdma");
	char buff[BUFFER_SIZE];
	tty->print_cr("Test 1");
	memset(buff, 0, BUFFER_SIZE);
	server = server_arr[0];
	tty->print_cr("RDMA write");
	memcpy(buff, "Dat", BUFFER_SIZE);
	server->rdma_write(buff, BUFFER_SIZE, 0);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	memset(buff, 0, BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("RDMA read");
	server->rdma_read(buff, BUFFER_SIZE, 0);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("-------------------------------");


	tty->print_cr("Test 2");
	memset(buff, 0, BUFFER_SIZE);
	server = server_arr[1];
	tty->print_cr("RDMA write");
	memcpy(buff, "Dat", BUFFER_SIZE);
	server->rdma_write(buff, BUFFER_SIZE, 0);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	memset(buff, 0, BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("RDMA read");
	server->rdma_read(buff, BUFFER_SIZE, 0);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("-------------------------------");


	tty->print_cr("Test 3");
	memset(buff, 0, BUFFER_SIZE);
	server = server_arr[0];
	tty->print_cr("RDMA write");
	memcpy(buff, "Nguyen", BUFFER_SIZE);
	server->rdma_write(buff, BUFFER_SIZE, 64);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	memset(buff, 0, BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("RDMA read");
	server->rdma_read(buff, BUFFER_SIZE, 64);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("-------------------------------");


	tty->print_cr("Test 4");
	memset(buff, 0, BUFFER_SIZE);
	server = server_arr[0];
	tty->print_cr("RDMA write");
	memcpy(buff, "Nguyen", BUFFER_SIZE);
	server->rdma_write(buff, BUFFER_SIZE, size_per_server() - BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	memset(buff, 0, BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("RDMA read");
	server->rdma_read(buff, BUFFER_SIZE, size_per_server() - BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("-------------------------------");


	tty->print_cr("Test 5");
	memset(buff, 0, BUFFER_SIZE);
	server = server_arr[0];
	tty->print_cr("RDMA write");
	memcpy(buff, "Nguyen", BUFFER_SIZE);
	server->rdma_write(buff, 1, size_per_server() - 1);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	memset(buff, 0, BUFFER_SIZE);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("RDMA read");
	server->rdma_read(buff, 1, size_per_server() - 1);
	print_a_buffer(buff, BUFFER_SIZE, "Test buff");
	tty->print_cr("-------------------------------");
	// rr->rdma_write(buff, 16, 0);

	// memset(buff, 0, BUFFER_SIZE);
	

	// tty->print_cr("RDMA read");
	// rr->rdma_read(buff, BUFFER_SIZE, 0);
	// print_a_buffer(buff, BUFFER_SIZE, (char*)"Test buff");
}

void RemoteMem::addr_to_pos(char* addr, size_t& server_idx, size_t& offset) {
	size_t remote_offset = addr - base();
	server_idx = remote_offset / size_per_server();

	assert(server_idx < num_connections, "must be a valid server");
	offset = remote_offset % size_per_server();
	assert(offset < size_per_server(), "must be a valid offset into the server");
}

void RemoteMem::record_new_tlab(size_t size_in_words) {
	shenandoah_assert_heaplocked();
	// if (_current_buffer_offset_in_words + size_in_words >= ShenandoahHeapRegion::region_size_words()) {
	// 	_heap->make_parsable(true);
	// 	tty->print_cr("verify buffer correctness");
	// 	ResourceMark rm;
	// 	oop obj = oop(_rdma_buff);
	// 	while (obj < (HeapWord*)_rdma_buff+ShenandoahHeapRegion::region_size_words()){
	// 		tty->print_cr("oop %p, klass %p", (void*)obj, (void*)obj->klass());
	// 		obj = oop ((HeapWord*)obj + obj->size());
	// 	}
	// }
	_current_buffer_offset_in_words = _current_buffer_offset_in_words + size_in_words;
	tty->print_cr("Buffer offset: %lu", _current_buffer_offset_in_words);
	// tty->print_cr("region size %lu", ShenandoahHeapRegion::region_size_words());
	// bool validity = _current_buffer_offset_in_words < ShenandoahHeapRegion::region_size_words();
	// tty->print_cr("Validity: %d", validity);
	// assert(true, "true assertion should be catched");

	assert(_current_buffer_offset_in_words < ShenandoahHeapRegion::region_size_words(), "Buffer overflow, need to do something at this point | buff_offset %lu, Region size %lu", _current_buffer_offset_in_words, ShenandoahHeapRegion::region_size_words());
}

void* RemoteMem::get_corresponding_evac_buffer_address(void* remote_addr) {
	assert(remote_addr, "address must not be null");
	assert(is_in(remote_addr), "Must be remote");
	assert(_rdma_buff, "rdma buffer must have been init");
	ShenandoahHeapRegion* rr = _heap->heap_region_containing(remote_addr);
	assert(rr->is_remote(), "Must be a remote region");
	assert(rr->evac_bottom(), "Evac bottom must be init");
	assert(is_in_evac_set(rr), "Region must be in evac set");
	size_t offset_from_evac_bottom = pointer_delta((HeapWord*)remote_addr, rr->evac_bottom());
	assert(offset_from_evac_bottom < ShenandoahHeapRegion::region_size_words(), "sanity");

	size_t region_evac_offset_into_buffer = rr->rdma_buffer_offset_in_words();

	size_t object_offset = region_evac_offset_into_buffer + offset_from_evac_bottom;
	return (HeapWord*)_rdma_buff + object_offset;
}

bool RemoteMem::is_in_evac_buff(void* addr) {
	return addr >= (void*)_rdma_buff && addr < (void*)(_rdma_buff + ShenandoahHeapRegion::region_size_bytes());
}

void RemoteMem::check_consecutive_oop(void* remote_start, size_t words) {
	tty->print_cr("check_consecutive_oop if tlab in region %lu: words %lu", _heap->heap_region_index_containing(remote_start), words);
	HeapWord* buff_start = (HeapWord*)get_corresponding_evac_buffer_address(remote_start);
	ResourceMark rm;
	oop obj = oop(buff_start);
	while (obj < buff_start + words){
		tty->print_cr("oop %p, klass %p %s, size %d", (void*)obj, (void*)obj->klass(), obj->klass()->external_name(), obj->size());
		obj = oop ((HeapWord*)obj + obj->size());
	}
	tty->print_cr("-------------------------------------------------");
}

void 	RemoteMem::add_region_to_evac_set (ShenandoahHeapRegion* r) {
	assert(!is_in(r), "Already in collection set");
	_evacuation_set[r->index()] = 1;
}
void	RemoteMem::remove_region_from_evac_set (ShenandoahHeapRegion* r) {
	assert(is_in(r), "Not in collection set");
	_evacuation_set[r->index()] = 0;
}

bool RemoteMem::is_in_evac_set(ShenandoahHeapRegion* r) {
	return is_in_evac_set(r->index());
}
bool RemoteMem::is_in_evac_set(size_t region_idx) {
	assert(region_idx < _heap->num_regions(), "Sanity");
	assert(_evacuation_set, "evac set must be init");
	return _evacuation_set[region_idx] == 1;
}
bool RemoteMem::is_in_evac_set(void* p) {
	size_t r_idx = _heap->heap_region_index_containing(p);
	return is_in_evac_set(r_idx);
}

void RemoteMem::clear_evac_set(){
	memset(_evacuation_set, 0, _heap->num_regions());
}

void RemoteMem::write(char* to_addr, char* from_buffer, size_t byte_length) {
	size_t server_idx = 0;
	size_t offset = 0;
	assert(_heap->is_in_remote(to_addr), "Must be a remote addr");

	addr_to_pos(to_addr, server_idx, offset);

	// tty->print_cr("Writing to server %lu, offset %lu", server_idx, offset);

	RDMAServer* server = server_arr[server_idx];
	server->rdma_write(from_buffer, byte_length, offset);
}

void RemoteMem::write(HeapWord* to_addr, HeapWord* from_buffer, size_t word_length) {
	write((char*)to_addr, (char*) from_buffer, word_length*HeapWordSize);
}

template <typename T>
void RemoteMem::write(T val, void* to_addr) {
	tty->print_cr("writing primitive");
	write((char*)to_addr, (char*) &val, sizeof(T));
}

void RemoteMem::write_obj_header(oopDesc header, void* to_addr) {
	tty->print_cr("writing object header");
	write((char*)to_addr, (char*) &header, sizeof(oopDesc));
}

void RemoteMem::read(char* from_addr, char* to_buffer, size_t byte_length) {
	size_t server_idx = 0;
	size_t offset = 0;
	assert(_heap->is_in_remote(from_addr), "Must be a remote addr");

	addr_to_pos(from_addr, server_idx, offset);

	// tty->print_cr("Reading to server %lu, offset %lu", server_idx, offset);

	RDMAServer* server = server_arr[server_idx];
	server->rdma_read(to_buffer, byte_length, offset);
}

void RemoteMem::read(HeapWord* from_addr, HeapWord* to_buffer, size_t word_length) {
	read((char*)from_addr, (char*) to_buffer, word_length*HeapWordSize);
}

template <typename T>
T RemoteMem::read(void* from_addr) {
	tty->print_cr("reading primitive");
	T ret_val;
	read((char*)from_addr, (char*) &ret_val, sizeof(T));
	return ret_val;
}

oopDesc RemoteMem::read_obj_header(void* from_addr) {
	tty->print_cr("reading obj header");
	oopDesc ret_val;
	read((char*)from_addr, (char*) &ret_val, sizeof(oopDesc));
	return ret_val;
}

uint64_t RemoteMem::remote_cas(char* addr, uint64_t expected, uint64_t desired) {
	// work only for 64 bit field
	// previous is to be returned. It is value of that field before cas.
	// if field equals expected, write desired to field
	// else, does nothing
	size_t server_idx = 0;
	size_t offset = 0;
	assert(_heap->is_in_remote(addr), "Must be a remote addr");
	addr_to_pos(addr, server_idx, offset);

	tty->print_cr("CAS to server %lu, offset %lu", server_idx, offset);

	RDMAServer* server = server_arr[server_idx];
	uint64_t previous = 0;
	server->atomic_cas(previous,offset, expected, desired);
	return previous;
}

// void RemoteMem::read_obj_header(void* from_addr, oopDesc* obj_addr) {
// 	if (UseCompressedClassPointers) {
// 		read((char*)from_addr, (char*) obj_addr, sizeof(oopDesc));
// 	} else {
// 		// last bit for 
// 		read((char*)from_addr, (char*) obj_addr, sizeof(oopDesc) + sizeof(int));
// 	}
// }

// int RemoteMem::send_comm_code(RDMAServer* server, COMM_CODE code) {
// 	int ret;
// 	int node_idx = server->idx;
//     // now we can close both process by posting a send

// 	tty->print_cr("rdma_post_send ...");
//     memset(send_msg, 0, BUFFER_SIZE);
// 	memcpy(send_msg, &node_idx, sizeof(node_idx));
//     memcpy(send_msg + sizeof(node_idx), &code, sizeof(COMM_CODE));
//     print_a_buffer(send_msg, BUFFER_SIZE, "send_msg");
// 	ret = rdma_post_send(server->cm_id, NULL, send_msg, 16, send_mr, send_flags);
// 	if (ret) {
// 		perror("rdma_post_send");
// 		disconnect(out_disconnect);
// 		// out_func(out_disconnect);
// 	}

//     tty->print_cr("rdma_get_send_comp ...");
// 	while ((ret = rdma_get_send_comp(server->cm_id, &wc)) == 0);
// 	if (ret < 0)
// 		perror("rdma_get_send_comp");
// 	else
// 		ret = 0;

//     return ret;
// }
