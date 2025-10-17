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

bool check_ptr (const void *ptr) 
{
  if (ptr == NULL || !is_user_vaddr (ptr) || pagedir_get_page (thread_current()->pagedir, ptr) == NULL)
  {
    thread_current()->exit_stat = -1;
    printf("%s: exit(%d)\n", thread_current()->name);
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
      char *executable = *temp;
      if (executable == NULL || executable == '\0') {
        thread_current()->exit_stat = -1;
        printf("%s: exit(%d)\n", thread_current()->name, -1);
        thread_exit();
      }

      char *s = executable;
      while (*s != '\0') {
        check_ptr(s);
        s++;
      }
      
      lock_acquire(&file_lock);
      f->eax = process_execute((const char*) executable);
      lock_release(&file_lock);
      break;
    }
    case SYS_WAIT: {
      temp = (int*) f->esp + 1;
      check_ptr(temp);
      f->eax = process_wait((tid_t) *temp);
      break;
    }
    case SYS_CREATE: {
      char **temp = (char **)f->esp + 1;
      int *temp2 = (int *) f->esp + 2;
      check_ptr(temp);
      check_ptr(temp2);
      
      char *file_name = *temp;
      if (file_name == NULL || file_name == '\0') {
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
      if (!temp || pagedir_get_page(thread_current()->pagedir, (const char*) *temp) == NULL) {
        thread_current() ->status = -1;
        thread_exit();
      }
      lock_acquire(&file_lock);
      f->eax = filesys_remove((const char*) *temp);
      lock_release(&file_lock);
      break;

    case SYS_OPEN:
      temp = (int*) f->esp + 1;
      if (!temp || pagedir_get_page(thread_current()->pagedir, (const char*) *temp) == NULL) {
        thread_current() ->status = -1;
        thread_exit();
      }
      lock_acquire(&file_lock);
      struct file *file = file_open((const char*) *temp);
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

    case SYS_FILESIZE:
      temp = (int*) f->esp + 1;
      lock_acquire(&file_lock);
      file_desc = find_filept (*temp);
      if (!file_desc) {
        f->eax = -1;
        lock_release(&file_lock);
        thread_exit();
      }
      f->eax = file_length(file_desc->file);
      lock_release(&file_lock);
      break;
      
    case SYS_READ:
      temp = (int*) f->esp + 1;
      temp2 = (int*) f->esp + 2;
      temp3 = (int*) f->esp + 3;
      if (!temp2 || pagedir_get_page(thread_current()->pagedir, (const void*)temp2) == NULL) {
        thread_current() ->status = -1;
        thread_exit();
      }
      if (!temp || !temp3) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc == NULL) {
        f->eax = -1;
        lock_release(&file_lock);
        thread_exit();
      }
      f->eax = file_read(file_desc->file, (void*) temp2, (int32_t) *temp3);
      lock_release(&file_lock);
      break;

    case SYS_WRITE: {
      int *t = (int*)f->esp + 1;
      void **t2 = (void**)(int*) f->esp + 2;
      unsigned *t3 = (unsigned*) f->esp + 3;
      check_ptr(t);
      check_ptr(t2);
      check_ptr(t3);
      if (*t == 1) {
        f->eax = *t3;
        putbuf(*t2, *t3);
        break;
      }
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc == NULL) {
        f->eax = -1;
        lock_release(&file_lock);
        break;
      }
      f->eax = file_write(file_desc->file, (const void*) temp2, (int32_t) *t3);
      lock_release(&file_lock);
      break;
    }
    case SYS_SEEK:
      temp = (int*)(f->esp) + 1;
      temp2 = (unsigned*)(int*)(f->esp) + 2;
      if (!temp2 || pagedir_get_page(thread_current()->pagedir, temp2) == NULL) {
        thread_current() ->status = -1;
        thread_exit();
      }
      if (!temp) {
        thread_exit();
      }
      lock_acquire(&file_lock);
      file_desc = find_filept(*temp);
      if (file_desc == NULL) {
        lock_release(&file_lock);
        thread_exit();
      }
      file_seek(file_desc->file, *temp2);
      lock_release(&file_lock);
      break;
      
    case SYS_TELL:
      temp = (int*)(f->esp) + 1;
      // if (!temp || pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
      //   thread_exit();
      // }
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
      // if (!temp || pagedir_get_page(thread_current()->pagedir, temp) == NULL) {
      //   thread_exit();
      // }
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
