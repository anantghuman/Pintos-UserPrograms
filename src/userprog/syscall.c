#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "pagedir.h"
#include "threads/synch.h"

struct lock file_lock;

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
      lock_acquire(&file_lock);
      int *temp = ((int*)f->esp) + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = exec(*((int*)f->esp + 1));
      lock_release(&file_lock);
      break;
    case SYS_WAIT:
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = process_wait((tid_t)*temp);
      break;
    case SYS_CREATE:
      lock_acquire(&file_lock);
      int *temp = ((int*)f->esp) + 1;
      int *temp2 = ((int*)f->esp) + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = filesys_create(*temp, *temp2);
      lock_release(&file_lock);
      break;
    case SYS_REMOVE:
      lock_acquire(&file_lock);
      int *temp = ((int*)f->esp) + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = remove(*temp);
      lock_release(&file_lock);
      break;
    case SYS_OPEN:
      lock_acquire(&file_lock);
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = open(*((int*)f->esp + 1));
      lock_release(&file_lock);
      break;
    case SYS_FILESIZE:
      lock_acquire(&file_lock);
      int *temp = f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = filesize(*((int *)f->esp + 1));
      lock_release(&file_lock);
      break;
    case SYS_READ:
      lock_acquire(&file_lock);
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
      lock_release(&file_lock);
      break;
    case SYS_WRITE:
      lock_acquire(&file_lock);
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
      lock_release(&file_lock);
      break;
    case SYS_SEEK:
      lock_acquire(&file_lock);
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = seek(*((int *)f->esp + 1), *((int *)f->esp + 2));
      lock_release(&file_lock);
    case SYS_TELL:
      lock_acquire(&file_lock);
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = tell(*((int *)f->esp + 1));
      lock_release(&file_lock);
      break;
    case SYS_CLOSE:
      lock_acquire(&file_lock);
      int *temp = f->esp + 1;
      int *temp2 = f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = close(*((int *)f->esp + 1));
      lock_release(&file_lock);
      break;
  }
  thread_exit ();
}
