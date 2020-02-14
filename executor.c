/* Haewon Han, 112433581, hhan43*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include "command.h"
#include "executor.h"

static void print_tree(struct tree *t);

int execute(struct tree *t) {  
  if (t->conjunction == NONE) {
      pid_t child_pid;

    /* cd and exit - internally impelmenting*/

    /* cd */
    if (strcmp(t->argv[0], "cd") == 0) {
      int status;
      if (t->argv[1] == NULL) {
	status = chdir(getenv("HOME"));
      } else {
	status = chdir(t->argv[1]);
      }

      if (status == -1) {
        printf("Failed to execute %s %s\n", t->argv[0], t->argv[1]);
	fflush(stdout);
	return 1;
      } else {
	return 0;
      }
    /* exit */
    } else if (strcmp(t->argv[0], "exit") == 0) {
      exit(1);

    /* for all other arguments, fork and execvp  */
    } else {

      if ((child_pid = fork()) < 0) {
	perror("fork error");
      }
      
      if (child_pid == 0) { /* child code */
	
	if (t->input != NULL) {
	  int fd;
	  if ((fd = open(t->input, O_RDONLY)) < 0) {
	    perror("File opening (read) failed");
	    exit(1);
	  } else {
	    dup2(fd, STDIN_FILENO);
	    close(fd);
	  }
	}

	if (t->output != NULL) {
	  int fd;
	  if ((fd = open(t->output, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0) {
	    perror("File opening (write) failed");
	    exit(1);
	  } else {
	    dup2(fd, STDOUT_FILENO);
	    close(fd);
	  } 
	}
	
	execvp(t->argv[0], t->argv);
	printf("Failed to execute %s\n", t->argv[0]);
	fflush(stdout);
	exit(1);
      } else { /* parent code */
	int status2;
	wait(&status2);
	return status2;
      }
      
    } 
    
  } else if (t->conjunction == AND) {

    int result;
    result = execute(t->left);
    /* Only do right command when left command returns 0 */
    if (result == 0) {
      result = execute(t->right);
    }
    return result;
    
  } else if (t->conjunction == OR) {

    int result;
    result = execute(t->left);
    /* Only do right command when left command returns NONZERO or -1 (fail)*/
    if (result != 0 && result != -1) {
      result = execute(t->right);
    }

    return result;
  } else if (t->conjunction == SEMI) {
    int result;
    result = execute(t->left);
    
    /* only do right when left is not -1 (fail)*/
    if (result != -1) {
      result = execute(t->right);
    }

    return result;
  } else if (t->conjunction == PIPE) {
   
    int pipe_fd[2];
    pid_t child_pid;
    /* open a pipe */
    if (pipe(pipe_fd) < 0) {
      err(EX_OSERR, "pipe erorr");
    }

    if ((child_pid = fork()) < 0) {
      err(EX_OSERR, "fork error");
    }

    if (child_pid) { /* parent reads its STDIN from pipe */
      int status1;
	
      close(pipe_fd[1]); /* closing pipe's write-end */

      /* Input redirecton: stdinput will be from read-end */
      if(dup2(pipe_fd[0], STDIN_FILENO) < 0) {
	err(EX_OSERR, "dup2 error");
      }
	
      close(pipe_fd[0]); /* closing pipe's read-end */
      execute(t->right);
      wait(&status1);

      if (status1 == 0) {
	return 0;
      } else {
	return 1;
      }
      exit(1);
    } else { /* child writes its STDOUT from pipe */

      close(pipe_fd[0]); /* closing pipe's read-end */

      /* output redirection: stdoutput will be written on write-end */
      if (dup2(pipe_fd[1], STDOUT_FILENO) < 0) {
	err(EX_OSERR, "dup2 error");
      }

      close(pipe_fd[1]); /* closing pipe's write-end */

      if(execute(t->left) == 0) {
	exit(0);
      } else {
	exit(1);
      }
      exit(1);
    }
  
  }

  return 0;
}


static void print_tree(struct tree *t) {
  if (t != NULL) {
    print_tree(t->left);

    if (t->conjunction == NONE) {
      printf("NONE: %s, ", t->argv[0]);
    } else {
      printf("%s, ", conj[t->conjunction]);
    }
    printf("IR: %s, ", t->input);
    printf("OR: %s\n", t->output);

    print_tree(t->right);
  }
}
