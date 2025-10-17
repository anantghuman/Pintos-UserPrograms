#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "pagedir.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

struct lock file_lock;

static void syscall_handler (struct intr_frame *);

void syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  lock_init (&file_lock);
}

void check_ptr (const void *ptr) 
{
  if (ptr == NULL || !is_user_vaddr (ptr) || pagedir_get_page (thread_current()->pagedir, ptr) == NULL)
  {
    thread_current()->exit_stat = -1;
    printf("%s: exit(-1)\n", thread_current()->name);
    thread_exit();
  }
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
  // printf ("system call!\n");
  check_ptr (f->esp);
  check_ptr ((const int*) f->esp + 3);
  int *temp;
  int *temp2;
  int *temp3;
  int syscall_number = *(int*)f->esp;
  struct file_descriptor *file_desc;
  switch (syscall_number) {
    case SYS_HALT:
      shutdown_power_off();
      break;

    case SYS_EXIT: {
      int* temp = (int *) f->esp + 1;
      check_ptr (temp);
      thread_current()->exit_stat = *temp;  
      printf("%s: exit(%d)\n", thread_current()->name, *temp);
      thread_exit();
      break;
    }
    case SYS_EXEC: {
      char **temp = (char **)(int*) f->esp + 1;
      check_ptr(temp);
      check_ptr(*temp);
      char *executable = *temp;
      if (executable == NULL || *executable == '\0') {
        f->eax = -1;
        thread_current()->exit_stat = -1;
        printf("%s: exit(%d)\n", thread_current()->name, -1);
        thread_exit();
      }

      for (char* t = executable; ; t++) {
        check_ptr(t);
        if (*t == '\0') {
          break;
        }
      }
      
      lock_acquire(&file_lock);
      f->eax = process_execute((const char*) executable);
      lock_release(&file_lock);
      break;
    }
    case SYS_WAIT: {
      int* temp = (int*) f->esp + 1;
      check_ptr(temp);
      f->eax = process_wait((tid_t) *temp);
      break;
    }
    case SYS_CREATE: {
      char **temp = (char **)f->esp + 1;
      int *temp2 = (int *) f->esp + 2;
      check_ptr(temp);
      check_ptr(*temp);
      check_ptr(temp2);
      
      char *file_name = *temp;
      if (file_name == NULL || *file_name == '\0') {
        thread_current()->exit_stat = -1;
        printf("%s: exit(%d)\n", thread_current()->name, -1);
        thread_exit();
      }
      
      char *t = file_name;
      while (*t != '\0') {
        check_ptr(t);
        t++;
      }

      lock_acquire(&file_lock);
      f->eax = filesys_create((const char*)*temp, (unsigned)*temp2);
      lock_release(&file_lock);
      break;
    }
    case SYS_REMOVE:
      int *temp = (int*) f->esp + 1;
      check_ptr(temp);
      check_ptr((const char*)*temp);
      lock_acquire(&file_lock);
      f->eax = filesys_remove((const char*) *temp);
      lock_release(&file_lock);
      break;

    case SYS_OPEN: {
      char **temp = (char **)(int*) f->esp + 1;
      check_ptr(temp);
      char *file_name = *temp;

      if (file_name == NULL) {
        f->eax = -1;
        break;
      }
      check_ptr(*temp);
      if (*file_name == '\0') {
        f->eax = -1;
        break;
      }

      char *ch = file_name;
      while (*ch != '\0') {
        check_ptr(ch);
        ch++;
      }
      check_ptr(file_name);
      lock_acquire(&file_lock);
      struct file *file = filesys_open((const char*) *temp);
      if (!file) {
        f->eax = -1;
        lock_release(&file_lock);
        break;
      }
      file_desc = malloc(sizeof(*file_desc));
      file_desc->num_fd = thread_current()->current_fd++;
      file_desc->file = file;
      f->eax = file_desc->num_fd;
      list_push_back(&thread_current()->fd_table, &file_desc->file_elem);
      f->eax = file_desc->num_fd;
      lock_release(&file_lock);
      break;
    }
    case SYS_FILESIZE:
      temp = (int*) f->esp + 1;
      check_ptr(temp);

      lock_acquire(&file_lock);
      file_desc = find_filept (*temp);
      if (!file_desc) {
        f->eax = -1;
      } else {
        f->eax = file_length(file_desc->file);
      }
      lock_release(&file_lock);
      break;
      
    case SYS_READ:
      temp = (int*) f->esp + 1;
      temp2 = (void**) ((int*) f->esp + 2);
      temp3 = (int*) f->esp + 3;
      check_ptr(temp);
      check_ptr(temp2);
      check_ptr(temp3);

      if (*temp3 < 0) {
        f->eax = -1;
        break;
      }
      if (*temp3 > 0) {
        check_ptr(*temp2);
      }
      if (*temp == 0) {
        uint8_t *t = (uint8_t*) *temp2;
        int i = 0;
        while (i < *temp3) {
          check_ptr(t + i);
          t[i] = input_getc();
          i++;
        }
        f->eax = *temp3;
        break;
      }

      if (*temp == 1) {
        f->eax = -1;
        break;
      }
    
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc == NULL) {
        f->eax = -1;
        lock_release(&file_lock);
        break;
      }
      f->eax = file_read(file_desc->file, *temp2, *temp3);
      lock_release(&file_lock);
      break;

    case SYS_WRITE: {
      int *t = (int*)f->esp + 1;
      void **t2 = (void**)(int*) f->esp + 2;
      unsigned *t3 = (unsigned*) f->esp + 3;
      check_ptr(t);
      check_ptr(t2);
      check_ptr(t3);
      
      if (*t3 > 0) {
        check_ptr(*t2);
      }

      if (*t == 1) {
        f->eax = *t3;
        putbuf((const char*)*t2, *t3);
        break;
      }

      if (*t == 0) {
        f->eax = -1;
        break;
      }

      lock_acquire(&file_lock);
      file_desc = find_filept(*t);
      if (file_desc == NULL) {
        f->eax = -1;
        lock_release(&file_lock);
        break;
      }
      f->eax = file_write(file_desc->file, *t2, *t3);
      lock_release(&file_lock);
      break;
    }
    case SYS_SEEK:
      temp = (int*)(f->esp) + 1;
      unsigned *t2 = (unsigned*)(int*)(f->esp) + 2;
      check_ptr(temp);
      check_ptr(t2);
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc != NULL) {
        file_seek(file_desc->file, *t2);
      } 
      lock_release(&file_lock);
      break;
      
    case SYS_TELL:
      temp = (int*)(f->esp) + 1;
      check_ptr(temp);
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc) {
        f->eax = file_tell(file_desc->file);
      } else {
        f->eax = -1;
      }
      lock_release(&file_lock);
      break;
    case SYS_CLOSE:
      temp = (int*)(f->esp) + 1;
      check_ptr(temp);
      if (*temp < 2) {
        break;
      }
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc) {
        file_close(file_desc->file);
        list_remove(&file_desc->file_elem);
        free(file_desc);
      }
      lock_release(&file_lock);
      break;
  }
  // thread_current()->status = -1;
  // thread_exit();
}
