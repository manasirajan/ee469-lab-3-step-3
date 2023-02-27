#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{
  int mc;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t no;
  mbox_t n2;
  mbox_t o2;
 
  mc = getpid();

  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 
  
  // Convert the command-line strings into integers for use as handles
  no = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  n2 = dstrtol(argv[2], NULL, 10);
  o2 = dstrtol(argv[3], NULL, 10);
  s_procs_completed = dstrtol(argv[4], NULL, 10);
  

 // Open the mailbox
 if (((mbox_open(no)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

  if (((mbox_open(n2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

  if (((mbox_open(o2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

  //receiving 2 NO
  if (mbox_recv(no, sizeof(int), (void *)&mc) == MBOX_FAIL) {
    Printf("[REACTION1]spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
  }
  if (mbox_recv(no, sizeof(int), (void *)&mc) == MBOX_FAIL) {
    Printf("[REACTION1]spawn_me (%d): Could not map the virtual address to the memory!\n", getpid());
    Exit();
  }

  //producing N2 + O2
  if (mbox_send(n2, sizeof(int), (void *)&mc) == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", n2, argv[0], getpid());
      Exit();
  }
  if (mbox_send(o2, sizeof(int), (void *)&mc) == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", n2, argv[0], getpid());
      Exit();
  }
 

  Printf("Reaction: 1 PID %d\n", getpid());

   if (((mbox_close(no)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

   if (((mbox_close(n2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

   if (((mbox_close(o2)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }
 
  // Now print a message to show that everything worked
  //Printf("injection 1 (%d): Received missile code: %c\n", getpid(), mc.really_important_char);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  //Printf("injection 1 (%d): Done!\n", getpid());



  // Signal the semaphore to tell the original process that we're done
  

}