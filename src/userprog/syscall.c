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
  int *temp;
  int *temp2;
  int *temp3;
  switch (syscall_number) {
    case SYS_HALT:
      shutdown_power_off();
      break;
    // check bounds for these
    case SYS_EXIT:
      temp = (int *) f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      thread_current()->status = temp;
      thread_exit();
      break;

    case SYS_EXEC:
      lock_acquire(&file_lock);
      temp = (const char*)*(((int*)f->esp) + 1);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = process_execute(temp);
      lock_release(&file_lock);
      break;

    case SYS_WAIT:
      temp = ((int*)(f->esp) + 1);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = process_wait((tid_t) *temp);
      break;

    case SYS_CREATE:
      lock_acquire(&file_lock);
      temp = (const char*) *(((int*)f->esp) + 1);
      temp2 = (unsigned) (((int*)f->esp) + 2);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = filesys_create(temp, *temp2);
      lock_release(&file_lock);
      break;

    case SYS_REMOVE:
      temp = (const char*) *(((int*)f->esp) + 1);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = filesys_remove(temp);
      lock_release(&file_lock);
      break;

    case SYS_OPEN:
      lock_acquire(&file_lock);
      temp = (const char*)*((int*)f->esp + 1);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = file_open(temp);
      list_push_back(&thread_current()->fd_table, f->eax);
      lock_release(&file_lock);
      break;

    case SYS_FILESIZE:
      lock_acquire(&file_lock);
      temp = (int*)f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = file_length(*temp);
      lock_release(&file_lock);
      break;
    case SYS_READ:
      lock_acquire(&file_lock);
      temp = (int*)f->esp + 1;
      temp2 = (void*)*((int*)f->esp + 2);
      temp3 = (unsigned*)((int*)f->esp + 3);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp3) == NULL) {
        thread_exit();
      }
      f->eax = file_read(*temp, temp2, *temp3);
      lock_release(&file_lock);
      break;

    case SYS_WRITE:
      lock_acquire(&file_lock);
      temp = (int*)f->esp + 1;
      temp2 = (const void*)*((int*) f->esp + 2);
      temp3 = (unsigned*)((int*) f->esp + 3);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp3) == NULL) {
        thread_exit();
      }
      f->eax = file_write(*temp, temp2, *temp3);
      lock_release(&file_lock);
      break;

    case SYS_SEEK:
      lock_acquire(&file_lock);
      temp = (int*)(f->esp) + 1;
      temp2 = (unsigned*)(int*)(f->esp) + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      f->eax = file_seek(*temp, *temp2);
      lock_release(&file_lock);
      break;
      
    case SYS_TELL:
      lock_acquire(&file_lock);
      temp = (int*)(f->esp) + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = file_tell(*temp);
      lock_release(&file_lock);
      break;
    case SYS_CLOSE:
      lock_acquire(&file_lock);
      temp = (int*)(f->esp) + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      f->eax = file_close(*temp);
      lock_release(&file_lock);
      break;
  }
  exit(-1);
}
