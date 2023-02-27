#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

// arrays for mboxes and mbox messages

static mbox mboxes[MBOX_NUM_MBOXES];
static mbox_message messages[MBOX_NUM_BUFFERS];


//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words,
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
  mbox_t i;
  //initialize(reset) the inuse parameter
  for(i = 0; i < MBOX_NUM_MBOXES; i++) {
    mboxes[i].inuse = 0;
  }
  for(i = 0; i < MBOX_MAX_BUFFERS_PER_MBOX; i++) {
    messages[i].inuse = 0;
  }
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use.
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {

  mbox_t i;
  cond_t cond1;
  cond_t cond2;
  lock_t the_lock;
  int j;
  uint32 intrval;

  if ((the_lock = LockCreate())== SYNC_FAIL){
    return MBOX_FAIL;
  };
  if ((cond1= CondCreate(the_lock))== SYNC_FAIL){
      return MBOX_FAIL;
    };
  if ((cond2= CondCreate(the_lock))== SYNC_FAIL){
      return MBOX_FAIL;
    };

  intrval = DisableIntrs();
  for(i = 0; i < MBOX_NUM_MBOXES; i++) {
  	if(mboxes[i].inuse == 0) {
	//find the earliest mbox not in use
		mboxes[i].inuse = 1;
    break;
    }
  }
  RestoreIntrs(intrval);
  if (i == MBOX_NUM_MBOXES){
     return MBOX_FAIL;
  }
		mboxes[i].cond1 = cond1;
		mboxes[i].cond2 = cond2;
		mboxes[i].lock = the_lock;
		//initialize the array of process
   		for(j = 0; j < PROCESS_MAX_PROCS; j++) {
			mboxes[i].process[j] = 0;
		}	
		return i;
	
  }
 


//-------------------------------------------------------
//
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle
// of the mailbox and the inuse flag will not be changed
// during execution.  This allows us to get the a valid
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
  if (LockHandleAcquire(mboxes[handle].lock) == SYNC_FAIL) {
    printf("Lock acquire fail\n");
    return MBOX_FAIL;
  }
  
  //add current process
  if(mboxes[handle].inuse == 0) {
    printf("mbox not in use\n");
    LockHandleRelease(mboxes[handle].lock);
    return MBOX_FAIL;
  }
  if (mboxes[handle].process[GetCurrentPid()] == 1){
    printf("Process already in mailbox\n"); 
    LockHandleRelease(mboxes[handle].lock);
    return MBOX_FAIL;}

  mboxes[handle].process[GetCurrentPid()] = 1;

  if (LockHandleRelease(mboxes[handle].lock) == SYNC_FAIL) {
    printf("Lock release fail\n");
    return MBOX_FAIL;
  }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  int j;
  uint32 intrval;
 Link * l;
  // do I need to disable interrupts
  if (mboxes[handle].inuse !=1){
    return MBOX_FAIL;
  }

  if(mboxes[handle].process[GetCurrentPid()] != 1){
    return MBOX_FAIL;
  }

 
  
  mboxes[handle].process[GetCurrentPid()] = 0;
  intrval = DisableIntrs();
  for (j = 0; j<PROCESS_MAX_PROCS; j++){
    if (mboxes[handle].process[j] != 0)
    break;
  }

  

if (j==PROCESS_MAX_PROCS){
   while(!AQueueEmpty(&mboxes[handle].msg)){
    if((l = AQueueLast(&mboxes[handle].msg)) == QUEUE_FAIL){
       RestoreIntrs(intrval);
      return MBOX_FAIL;
    }
    if((l = AQueueRemove(&l))== QUEUE_FAIL){
       RestoreIntrs(intrval);
      return MBOX_FAIL;
    }

  }
mboxes[handle].inuse = 0; 
}

 RestoreIntrs(intrval);
    
  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {

  uint32 intrval;
  int i;
  Link * l;
  //Acquire the lock;
  if (LockHandleAcquire(mboxes[handle].lock) == SYNC_FAIL) {
    printf("Lock acquire fail\n");
    return MBOX_FAIL;
  }
  //Check if process has the mailbox open
  if (mboxes[handle].process[GetCurrentPid()]==0){
    LockHandleRelease(mboxes[handle].lock);
     printf("Not opened send fail\n");
    return MBOX_FAIL;
  }
  //Check message length
  if (length > MBOX_MAX_MESSAGE_LENGTH || length < 0){
    LockHandleRelease(mboxes[handle].lock);
     printf("Length fail\n");
    return MBOX_FAIL;
  }

  //check cond
  //do we need to use MBOX_NUM_BUFFERS?

  if(AQueueLength(&mboxes[handle].msg) >= MBOX_MAX_BUFFERS_PER_MBOX) {
    if(CondHandleWait(mboxes[handle].cond1) == SYNC_FAIL) {
      LockHandleRelease(mboxes[handle].lock);
       printf("Queue full \n");
      return MBOX_FAIL;
    }
  }
/*
if(CondWait(&mboxes[handle].cond1) == SYNC_FAIL) {
      return MBOX_FAIL;
    }*/

  intrval = DisableIntrs(); 

  //get free mbox message buffer
  //store message to mbox message array
  for(i = 0; i < MBOX_MAX_BUFFERS_PER_MBOX; i++) {
    //find the earliest mbox message available
    if(messages[i].inuse == 0) {
      messages[i].inuse = 1;    //set the mbox_message buffer to be in use
      
      break;
    }
  }
  //printf("%d\n", i);
	
  if (i == MBOX_MAX_BUFFERS_PER_MBOX + 1) {
      //no mbox_messages available
      RestoreIntrs(intrval);
      LockHandleRelease(mboxes[handle].lock);
       printf("No mbox\n");
      return MBOX_FAIL;
  }

 
bcopy(message, messages[i].message, length);

messages[i].length = length;

 RestoreIntrs(intrval);
	
if ((l = AQueueAllocLink((void *) &messages[i])) == NULL){
  printf("Queue link not working\n");
  LockHandleRelease(mboxes[handle].lock);
	return MBOX_FAIL;
}
  //insert message to queue - if we insert last here - retrieve first in receive
  if (AQueueInsertLast(&mboxes[handle].msg, l) == QUEUE_FAIL){
     printf("Couldn't insert\n");
     LockHandleRelease(mboxes[handle].lock);
	  return MBOX_FAIL;
  }

  //condSignal(notEmpty)
  if(CondHandleSignal(mboxes[handle].cond2) == SYNC_FAIL) {
    LockHandleRelease(mboxes[handle].lock);
    printf("Couldn't signal empty\n");
    return MBOX_FAIL;
  };

  //release lock
  if (LockHandleRelease(mboxes[handle].lock) == SYNC_FAIL) {
    printf("Lock release fail\n");
    return MBOX_FAIL;
  }

  return MBOX_SUCCESS;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox
// via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
//question -- are the condition variables for the queue or the message array? should be array since queue is not size constrained but then why is it a global array?
int MboxRecv(mbox_t handle, int maxlength, void* message) {
uint32 intrval;
mbox_message * link_object;
Link * l;
//char messagestring[MBOX_MAX_MESSAGE_LENGTH];
int messagesize;
//int i;
  //acquire lock
  if (LockHandleAcquire(mboxes[handle].lock) == SYNC_FAIL) {
    printf("Lock acquire fail\n");
    return MBOX_FAIL;
  }
  //Check if process has the mailbox open
  if (mboxes[handle].process[GetCurrentPid()]==0){
    LockHandleRelease(mboxes[handle].lock); 
    return MBOX_FAIL;
  }

  //check input message length
  if(maxlength > MBOX_MAX_MESSAGE_LENGTH || maxlength < 0) {
    LockHandleRelease(mboxes[handle].lock); 
    return MBOX_FAIL;
  }

  //check cond - if there is no message in the buffer?
	
  if(AQueueLength(&mboxes[handle].msg) == 0) {
    if(CondHandleWait(mboxes[handle].cond2) == SYNC_FAIL) {
      LockHandleRelease(mboxes[handle].lock); 
      return MBOX_FAIL;
    }
  }
/*
if(CondWait(&mboxes[handle].cond2) == SYNC_FAIL) {
      return MBOX_FAIL;
    }*/

  //get the message from the queue  
   //intrval = DisableIntrs(); 
/*
   for(i = 0; i < MBOX_MAX_BUFFERS_PER_MBOX; i++) {
    //find the earliest mbox message available
    if(messages[i].inuse == 1) {
      messages[i].inuse = 0;    //set the mbox_message buffer to be in use
      
      break;
    }
  }*/
  // RestoreIntrs(intrval);
   l = AQueueFirst(&mboxes[handle].msg);
   if (l== NULL){
    printf("[REACTION1] QUEUE IS EMPTY\n");
    LockHandleRelease(mboxes[handle].lock); 
	   return MBOX_FAIL;
   }
   link_object = (mbox_message *) AQueueObject(l);
   if (link_object == NULL){
     LockHandleRelease(mboxes[handle].lock); 
	   return MBOX_FAIL;
   }
	   
   messagesize = link_object->length;
   if (messagesize > maxlength){
    LockHandleRelease(mboxes[handle].lock); 
	  return MBOX_FAIL;
   }
	
   bcopy(link_object->message, message, messagesize);
   //printf("%c\n\n", link_object->message[0]);
   
   intrval = DisableIntrs(); 
   link_object->inuse = 0;
   RestoreIntrs(intrval);

	
//remove from queue
 if(AQueueRemove (&l) == QUEUE_FAIL){
   LockHandleRelease(mboxes[handle].lock); 
	 return MBOX_FAIL;
 }

  //condsignal(notFull)
  if(CondHandleSignal(mboxes[handle].cond1) == SYNC_FAIL) {
    LockHandleRelease(mboxes[handle].lock); 
    return MBOX_FAIL;
  };

  //release lock
  if (LockHandleRelease(mboxes[handle].lock) == SYNC_FAIL) {
    printf("Lock release fail\n");
    return MBOX_FAIL;
  }

  return messagesize;
}

//--------------------------------------------------------------------------------
//
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
	uint32 intrval;

  int i;
int j;
Link *l;
  intrval = DisableIntrs();
	for (i = 0; i < MBOX_NUM_MBOXES; i++){ //checking each mailbox
		if (mboxes[i].inuse == 0 && mboxes[i].process[pid] == 1){
			RestoreIntrs(intrval);
			return SYNC_FAIL;}
		if (mboxes[i].inuse ==0){
			continue;
		}
		mboxes[i].process[pid] = 0;
		for (j = 0; j < PROCESS_MAX_PROCS; j++){
			if (mboxes[i].process[j] == 1){
			break;
			}
		if (j==PROCESS_MAX_PROCS){
      while(!AQueueEmpty(&mboxes[i].msg)){
    if((l = AQueueLast(&mboxes[i].msg)) == QUEUE_FAIL){
       RestoreIntrs(intrval);
      return MBOX_FAIL;
    }
    if((l = AQueueRemove(&l))== QUEUE_FAIL){
       RestoreIntrs(intrval);
      return MBOX_FAIL;
    }

  }
			mboxes[i].inuse = 0;
	         }
		}
	}
  RestoreIntrs(intrval);
  return MBOX_SUCCESS;
}