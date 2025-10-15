#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "pagedir.h"

static Lock file_lock;

static void syscall_handler (struct intr_frame *);

void syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&file_lock);
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
      file_lock.acquire();
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = exec(*((int*)f->esp + 1));
      file_lock.release();
      break;
    case SYS_WAIT:
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = wait(*((int *)f->esp + 1));
      break;
    case SYS_CREATE:
      file_lock.acquire();
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = create(*((int*)f->esp + 1), *((int*)f->esp + 2));
      file_lock.release();
      break;
    case SYS_REMOVE:
      file_lock.acquire();
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = remove(*((int*)f->esp + 1));
      file_lock.release();
      break;
    case SYS_OPEN:
      file_lock.acquire();
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = open(*((int*)f->esp + 1));
      file_lock.release();
      break;
    case SYS_FILESIZE:
      file_lock.acquire();
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = filesize(*((int *)f->esp + 1));
      file_lock.release();
      break;
    case SYS_READ:
      file_lock.acquire();
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      int *temp3 = f->esp + 3;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp3) == NULL) {
        thread_exit();
      }
      f->eax = read(*((int *)f->esp + 1), *((int *)f->esp + 2), *((int *)f->esp + 3));
      file_lock.release();
      break;
    case SYS_WRITE:
      file_lock.acquire();
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      int *temp3 = f->esp + 3;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp3) == NULL) {
        thread_exit();
      }
      f->eax = write(*((int *)f->esp + 1), *((int *)f->esp + 2), *((int *)f->esp + 3));
      file_lock.release();
      break;
    case SYS_SEEK:
      file_lock.acquire();
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = seek(*((int *)f->esp + 1), *((int *)f->esp + 2));
      file_lock.release();
    case SYS_TELL:
      file_lock.acquire();
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = tell(*((int *)f->esp + 1));
      file_lock.release();
      break;
    case SYS_CLOSE:
      file_lock.acquire();
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = close(*((int *)f->esp + 1));
      file_lock.release();
      break;
  }
  thread_exit ();
}
