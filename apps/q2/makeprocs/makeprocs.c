#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 5;               // Used to store number of processes to create
  int num_no;
  int num_h2o;
  mbox_t no;
  mbox_t n2;
  mbox_t o2;
  mbox_t h2o;
  mbox_t h2;
  mbox_t hno3;

  int i;                          // Loop index variable
                   // Used to hold handle to shared memory page
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
            
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
  char no_completed_str[10];
  char n2_completed_str[10];
  char o2_completed_str[10];
  char h2o_completed_str[10];
  char h2_completed_str[10];
  char hno3_completed_str[10];
  //char num_no_completed_str[10];
  //char num_h2o_completed_str[10];
  //char no_h2o_completed_str[10];
  
  //chemicals created
  int num_h2_created;
  int num_n2_created;
  int num_o2_created;
  //chemicals left
  int num_no_left;
  int num_n2_left;
  int num_h2o_left;
  int num_h2_left;
  int num_o2_left;
  int num_hno3_left;
  //number of rxn 3 occurences
  int num_rxn_3;

  

  //run_os_tests(); // why is this here

  if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  //get number of NO molecules
  num_no = dstrtol(argv[1], NULL, 10);

  //get number of H2O molecules 
  num_h2o = dstrtol(argv[2], NULL, 10);


  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  /*
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  // Map shared memory page into this process's memory space
  if ((buff = (buffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }
  // Put some values in the shared memory, to be read by other processes
  buff->head = 0;
  buff->tail = 0;
  */

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  

  if (((no = mbox_create()) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((n2 = mbox_create()) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((o2 = mbox_create()) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((h2o = mbox_create()) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((h2 = mbox_create()) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((hno3 = mbox_create()) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

//opening the mailboxes
  if (((mbox_open(no)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((mbox_open(n2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if ((( mbox_open(o2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if ((( mbox_open(h2o)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if ((( mbox_open(h2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((mbox_open(hno3)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  //ditoa(h_mem, h_mem_str);
 
  ditoa(no, no_completed_str);
  ditoa(n2, n2_completed_str);
  ditoa(o2, o2_completed_str);
  ditoa(h2o, h2o_completed_str);
  ditoa(h2, h2_completed_str);
  ditoa(hno3, hno3_completed_str);
  /*
  ditoa(num_no/2, num_no_completed_str);
  ditoa(num_h2o/2, num_h2o_completed_str);
  */
  //find how many times reaction 3 occurs
  num_h2_created = (num_h2o / 2) *2;
  num_n2_created = num_no / 2;
  num_o2_created = num_no / 2 + num_h2o / 2;
  num_o2_created /= 3;    //divide by 3 due to rxn 3 equation
  //find the limiting reactant
  if(num_h2_created <= num_n2_created && num_h2_created <= num_o2_created) {
    //h2 is limiting reactant
    //ditoa(num_h2_created, no_h2o_completed_str);
    num_rxn_3 = num_h2_created;
  }
  else if(num_n2_created <= num_h2_created && num_n2_created <= num_o2_created) {
    //n2 is limiting reactant
    //ditoa(num_n2_created, no_h2o_completed_str);
    num_rxn_3 = num_n2_created;
  }
  else {
    //o2 is limiting reactant
    //ditoa(num_o2_created, no_h2o_completed_str);
    num_rxn_3 = num_o2_created;
  }
  
numprocs = dstrtol(argv[1], NULL, 10) + dstrtol(argv[2], NULL, 10) + num_no/2 + num_h2o/2 + num_rxn_3;
if ((s_procs_completed = sem_create(-(numprocs - 1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
   ditoa(s_procs_completed, s_procs_completed_str);
   //Printf("num procs %d\n", numprocs);
  
/*
  Printf("no completed %s makeprocs\n", no_completed_str);
  Printf("%s makeprocs\n", h2_completed_str);
   Printf("%s makeprocs\n", h2o_completed_str);
    Printf("%s makeprocs\n", n2_completed_str);
     Printf("%s makeprocs\n", o2_completed_str);
      Printf("%s makeprocs\n", hno3_completed_str);
    Printf("%s makeprocs\n", s_procs_completed_str);
  */
  // ditoa(mutex, mutex_str);
  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
for(i = 0; i < dstrtol(argv[1], NULL, 10); i++) {
    process_create(INJECTION1, 0, 0, no_completed_str, s_procs_completed_str, NULL);
}
for(i = 0; i < dstrtol(argv[2], NULL, 10); i++) {
    process_create(INJECTION2, 0, 0, h2o_completed_str, s_procs_completed_str, NULL);
}

for(i = 0; i < num_no/2; i++) {
    process_create(PROCESS1, 0, 0, no_completed_str, n2_completed_str, o2_completed_str, s_procs_completed_str, NULL);
}

for(i = 0; i < num_h2o/2; i++) {
    process_create(PROCESS2, 0, 0, h2o_completed_str, h2_completed_str, o2_completed_str, s_procs_completed_str, NULL);
}
for(i = 0; i < num_rxn_3; i++) {
    process_create(PROCESS3, 0, 0, h2_completed_str, n2_completed_str, o2_completed_str, hno3_completed_str, s_procs_completed_str, NULL);
}



  // And finally, wait until all spawned processes have finished.
  

  //calculating remaining left
  num_no_left = num_no % 2;   //number of NO left is either 1 or 0. (0 if num_no is even, 1 if num_no is odd)
  num_n2_left = num_no / 2 - (num_no + num_h2o)/6;  //number of N2 left is number of reaction 1 - reaction 3
  num_h2o_left = num_h2o % 2; //number of H2O left is either 1 or 0. (0 if num_no is even, 1 if num_no is odd)
  num_h2_left = 2 * num_h2o / 2 - num_rxn_3; //number of H2 left is 2 * reaction 2 - reaction 3
  num_o2_left = num_no / 2 + num_h2o / 2 - 3 * num_rxn_3;    //number of O2 left is reaction 1 + reaction 2 - 3* reaction 3
  num_hno3_left = 2 * num_rxn_3; //number of HNO3 is 2 * number of reaction 3 occurences

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
      Exit();
  }
//closing mailboxes
  if (((mbox_close(no)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((mbox_close(n2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if ((( mbox_close(o2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if ((( mbox_close(h2o)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if ((( mbox_close(h2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  if (((mbox_close(hno3)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
  Printf("Remain: %d NO, %d N2, %d H2O, %d H2, %d O2, %d HNO3\n", num_no_left, num_n2_left, num_h2o_left, num_h2_left, num_o2_left, num_hno3_left); 
}