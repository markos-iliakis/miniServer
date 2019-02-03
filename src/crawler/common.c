#include "common.h"
#include "stdio.h"
#include <sys/types.h>
#include "string.h"

int make_fifo_name(pid_t pid, char* name, int flag){
	char c[20];
	snprintf(c, 20, "%ld", (long)pid);

	int name_max = strlen(c) + 6;

	// read and write names are acording to worker
	if (flag)
		snprintf(name, name_max, "fifo%ldr", (long)pid);
	else
		snprintf(name, name_max, "fifo%ldw", (long)pid);

	return 1;
}

int mywrite(int fd, char *buff, size_t size){
	int rc;
	size_t done, todo;

	for (done=0; done < size; ) {
	    todo = size - done;
	    rc = write (fd, buff+done, todo);
	    switch (rc) {
	    case -1: /* some read error: check it */
			perror("jex : writing");
	        switch(errno) {
	        case EINTR: continue;
	        /* ... maybe some other cases you need to handle */
	        default: return -1;
	            }
	        break;
	    case 0: /* (in some cases) the other side closed the connection */
	        /* handle it here; possibly return error */
			perror("jex : writing");
	        break;
	    default: /* the normal case */
	        done += rc;
	        break;
	        }
	    }
	return done;
}

char* tokenize(char** buffer, char del){
	int i = 0;
	if((*buffer) == NULL)
		return NULL;
	while((*buffer)[i] != '\0'){
		if((*buffer)[i] == del){
			(*buffer)[i] = '\0';
			char* token = *buffer;
			(*buffer) += i+1;
			return token;
		}
		i++;
	}

	if(i==0) return NULL;
	char* token = *buffer;
	(*buffer) += i;

	if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0';
	return token;
}

// answer* answerInit(){
// 	answer* a = malloc(sizeof(answer));
// 	a->line_num =
// }
