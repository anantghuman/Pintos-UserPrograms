#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f UNUSED)
{
  printf ("system call!\n");
  int syscall_number = f->esp;
  switch (syscall_number) {
    case SYS_HALT:
      halt();
      break;
    // check bounds for these
    case SYS_EXIT:
      break;
    case SYS_EXEC:
      // exec()
      break;
    case SYS_WAIT:
      wait(*(int *)f->esp + 1);
      break;
    case SYS_CREATE:
      break;
    case SYS_REMOVE:
      break;
    case SYS_OPEN:
      break;
    case SYS_FILESIZE:
      filesize(*(int *)f->esp + 1);
      break;
    case SYS_READ:
      read(*(int *)f->esp + 1, *(int *)f->esp + 2, *(int *)f->esp + 3);
      break;
    case SYS_WRITE:
      write(*(int *)f->esp + 1, *(int *)f->esp + 2, *(int *)f->esp + 3);
      break;
    case SYS_SEEK:
      seek(*(int *)f->esp + 1, *(int *)f->esp + 2);
    case SYS_TELL:
      tell(*(int *)f->esp + 1);
      break;
    case SYS_CLOSE:
      close(*(int *)f->esp + 1);
      break;
  }
  thread_exit ();
}
