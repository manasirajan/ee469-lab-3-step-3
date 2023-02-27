#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{
  int mc;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  mbox_t no;
 

  mc = getpid();

  if (argc != 3) { 
    
    Printf("Usage:"); Printf("%d ", argc);Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 
  
  
  // Convert the command-line strings into integers for use as handles
  no = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed = dstrtol(argv[2], NULL, 10);
  /*
  for(i = 0; i < dstrtol(argv[3], NULL, 10); i++) {
    sem_signal(no); 
    Printf("Injection: NO PID %d\n", Getpid());
  }
  */

 // Open the mailbox
 if (((mbox_open(no)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

  // Wait for a message from the mailbox
  if (mbox_send(no, sizeof(int), (void *)&mc) == MBOX_FAIL) {
      Printf("Could not send message to mailbox %d in %s (%d)\n", no, argv[0], getpid());
      Exit();
  }
  Printf("Injection: NO PID %d\n", getpid());
 
  // Now print a message to show that everything worked
  //Printf("injection 1 (%d): Received missile code: %c\n", getpid(), mc.really_important_char);
  if (((mbox_close(no)) == MBOX_FAIL)) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();  	
  }

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  //Printf("injection 1 (%d): Done!\n", getpid());



  // Signal the semaphore to tell the original process that we're done
  

}