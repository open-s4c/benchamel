// args: {NUMTHREADS}
/********************************************************
*                                                       *
*     Copyright (C) Microsoft. All rights reserved.     *
*                                                       *
********************************************************/

#include <assert.h>
#define USE_NON_BLOCKING_SYNC
#include "WorkStealQueueWithState.h"
#include <iostream>
#include <strstream>

#include <pthread.h>
#include <cstring>

using namespace THE;

#define INITQSIZE 2 // must be power of 2

int nStealers = 2;
int nItems = 4;
int nStealAttempts = 2;

int args(int argc, char** argv){
	if(argc > 1){
		int arg1 = atoi(argv[1]);
		if(arg1 > 0){
			nStealers = arg1;
		}
	}
	if(argc > 2){
		int arg2 = atoi(argv[2]);
		if(arg2 > 0){
			nItems = arg2;
		}
	}
	if(argc > 3){
		int arg3 = atoi(argv[3]);
		if(arg3 > 0){
			nStealAttempts = arg3;
		}
	}
	std::cout << "\nWorkStealQueue Test: "
		<< nStealers << " stealers, "
		<< nItems << " items, "
		<< "and "
		<< nStealAttempts << " stealAttempts"
		<< std::endl;
	return 0;
}

int num;

class ObjType {
public:
	ObjType() { field = 0; id = ++num; }
	void Operation() { field++; }
	void Check() { assert(field == 1); }
	int id;

// private:
public:
	int field;
};

WorkStealQueue<ObjType *> *queue;

void* Stealer(void* param){
	int tid = (int) (size_t) param;
	state.Push(tid, F_Stealer);

	WorkStealQueue<ObjType*> *q = queue;
	ActivationRecord& ar = state.Peek(tid);

	ObjType* r;
	for(int i=0; i<nStealAttempts; i++){
		// pc == 0
		state.Peek(tid).Update(0, 0, (void *)(uintptr_t) i);
		if (q->Steal(r, tid))
			r->Operation();
	}

	state.Pop(tid);
	return 0;
}

int main(int argc, char** argv){
  args(argc, argv);
	int tid = 0;
	state.Push(tid, F_RunTest);

	pthread_t* handles = new pthread_t[nStealers];
  ObjType* items = new ObjType[nItems];

	WorkStealQueue<ObjType*> *q = new WorkStealQueue<ObjType*>(INITQSIZE);
	queue = q;
	num = 0;

	ActivationRecord& ar = state.Peek(tid);

	for (int i = 0; i < nStealers; i++) {
		pthread_create(&handles[i], NULL, Stealer, (void*)(uintptr_t)(i+1));
	}

	for (int i = 0; i < nItems/2; i++) {
		// pc == 1
		state.Peek(tid).Update(1, 0, (void *)(uintptr_t) i);
		q->Push(&items[2*i], tid);

		// pc == 2
		state.Peek(tid).Update(2, 0, (void *)(uintptr_t) i);
		q->Push(&items[2*i+1], tid);

		ObjType* r;
		// pc == 3
		state.Peek(tid).Update(3, 0, (void *)(uintptr_t) i);
		if (q->Pop(r, tid))
			r->Operation();
	}

	for (int i = 0; i < nItems/2; i++) {
		ObjType* r;
		// pc == 4
		state.Peek(tid).Update(4, 0, (void *)(uintptr_t) i);
		if (q->Pop(r, tid))
			r->Operation();
	}

	// pc == 5
	state.Peek(tid).Update(5);
	for (int i = 0; i < nStealers; i++) {
    pthread_join(handles[i], NULL);
  }

	for (int i = 0; i < nItems; i++) {
		items[i].Check();
	}

	delete[] items;
	delete[] handles;
	delete q;

	state.Pop(tid);
	state.Clear();
	return 0;
}

int GetState(char* buf, int len)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	std::strstream o;
#pragma GCC diagnostic pop
	o << "Q";

	// Write queue
	if(queue != 0){
		o << queue->lock << queue->mask;
		o << queue->ComputeIndex(queue->tail);

		for (int i = 0; queue->head+i < queue->tail; i++)
		{
			o << queue->elems[(queue->head+i)&queue->mask]->id;
			o << queue->elems[(queue->head+i)&queue->mask]->field;
		}
	}

	// Write state
	for (size_t i = 0; i < state.stacks.size(); i++)
	{
		std::vector<ActivationRecord> stack = state.stacks[i];
		for (size_t j = 0; j < stack.size(); j++)
		{
			ActivationRecord ar = stack[j];
			o << ar.fname << ar.pc;
			for (size_t k = 0; k < ar.lvars.size(); k++)
			{
				o << ar.lvars[k];
			}
		}
	}
	o << std::endl;

	int ret = o.pcount();
	if(len > o.pcount()){
		len = o.pcount();
	}
	memcpy(buf, o.str(), len);
	return ret;
}
