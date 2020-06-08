#include <sys/inotify.h>
#include <stdio.h>
#include <unistd.h>
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int main (int argc, char *argv[]) { 
	int fd, wd;
	char buff[BUF_LEN];
	int len, i = 0;

	fd = inotify_init();
	if(fd < 0) perror("inotify_init");

	wd = inotify_add_watch(fd, 
					"/home/ulises/Documents/watch",
					IN_MODIFY | IN_CREATE | IN_DELETE);

	if(wd < 0) perror("inotify_add_watch");
	while(1){
		i=0;
		len = read(fd, buff, BUF_LEN);
		while(i < len){
			struct inotify_event *event;
			event = (struct inotify_event *) &buff[i];
			printf ("wd=%d mask=%u cookie=%u len=%u\n",
	                event->wd, event->mask,
	                event->cookie, event->len);

	        if (event->len)
	                printf ("name=%s\n", event->name);

	        i += EVENT_SIZE + event->len;
		}
	}
	return 0;
}