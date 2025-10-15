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

struct file_descriptor *find_filept(int fd) {
  struct thread *temp = thread_current();
  struct list_elem *i = list_begin(&temp->fd_table);
  while (i != list_end(&temp->fd_table)) {
    struct file_descriptor *file_desc = list_entry(i, struct file_descriptor, file_elem);
    if (file_desc->num_fd == fd) {
      return file_desc;
    }
    i = list_next(i);
  }
  return NULL;
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
    case SYS_EXIT:
      temp = (int *) f->esp + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      thread_current()->status = temp;
      thread_exit();
      break;

    case SYS_EXEC:
      temp = *((int*)f->esp) + 1;
      if (!temp || pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = process_execute((const char*)temp);
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
      temp = *((int*)f->esp) + 1;
      temp2 = (int*)f->esp + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = filesys_create((const char*) temp, (unsigned)*temp2);
      lock_release(&file_lock);
      break;

    case SYS_REMOVE:
      temp = *(((int*)f->esp) + 1);
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = filesys_remove((const char*)temp);
      lock_release(&file_lock);
      break;

    case SYS_OPEN:
      temp = (const char*)*((int*)f->esp + 1);
      if (!temp || pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      struct file *file = file_open(temp);
      struct file_descriptor *file_desc;
      file_desc->num_fd = thread_current()->current_fd;
      thread_current ()->current_fd++;
      file_desc->file = file;
      lock_acquire(&file_lock);
      list_push_back(&thread_current()->fd_table, &file_desc->file_elem);
      lock_release(&file_lock);
      break;

    case SYS_FILESIZE:
      temp = (int*)f->esp + 1;
      if (!temp || pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      struct file *file = find_filept (*temp)->file;
      lock_acquire(&file_lock);
      f->eax = file_length(file);
      lock_release(&file_lock);
      break;
      
    case SYS_READ:
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
      lock_acquire(&file_lock);
      f->eax = file_read(*temp, temp2, *temp3);
      lock_release(&file_lock);
      break;

    case SYS_WRITE:
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
      lock_acquire(&file_lock);
      f->eax = file_write(*temp, temp2, *temp3);
      lock_release(&file_lock);
      break;

    case SYS_SEEK:
      temp = (int*)(f->esp) + 1;
      temp2 = (unsigned*)(int*)(f->esp) + 2;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      if (pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = file_seek(*temp, *temp2);
      lock_release(&file_lock);
      break;
      
    case SYS_TELL:
      temp = (int*)(f->esp) + 1;
      if (pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = file_tell(*temp);
      lock_release(&file_lock);
      break;
    case SYS_CLOSE:
      temp = (int*)(f->esp) + 1;
      if (!temp || pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      struct file_descriptor *file_desc = find_filept(*temp);
      if (!file_desc) {
        lock_release(&file_lock);
        break;
      }
      file_close(file_desc->file);
      list_remove(&file_desc->file_elem);
      lock_release(&file_lock);
      break;
  }
  exit(-1);
}
