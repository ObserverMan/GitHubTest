#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

inline void print_usage(void)
{
	fprintf(stderr,"Usage: readfile [path]\n");
}

int main(int argc, char *argv[])
{
	int fd;
	size_t buff_size;
	size_t max_buff_size;
	char *buff;
	char *old_buff_ptr;
	ssize_t read_len;
	ssize_t i;
	unsigned long  total_read;
	struct tm *tm;
	time_t _time;


	if (argc < 2)
	{
		print_usage();
		return -1;
	}

	fprintf(stdout, "size of buff_size: %d PAGE_SIZE = %ld\n", sizeof(buff_size), PAGE_SIZE);

	while (1)
	{
		fd = open(argv[1], O_RDONLY);

		if (-1 == fd)
		{
			fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
			print_usage();
			return -1;
		}

		max_buff_size = SIZE_MAX;
		buff_size = (1 << 22) < max_buff_size ? (1 << 22) : max_buff_size; 
		total_read = 0;
		buff = NULL;

		while (1)
		{
			old_buff_ptr = buff;
			buff = realloc(old_buff_ptr, buff_size);
			if (NULL == buff)
			{
				fprintf(stderr, "Error, failed to malloc on size %ld\n", buff_size);

				if (old_buff_ptr != NULL)
				{
					free(old_buff_ptr);
					old_buff_ptr = NULL;
				}

				max_buff_size = buff_size >> 1;
				if (max_buff_size == 0)
					break;

				buff_size = max_buff_size >> 2;
				continue;
			}
			else
			{
				_time = time(NULL);
				tm = localtime(&_time);
				fprintf(stdout, "%d:%d:%d Realloc %ld bytes(%ldMB) %ld pages\n",
						tm->tm_hour, tm->tm_min, tm->tm_sec,
						buff_size, buff_size / 1024 / 1024, buff_size / PAGE_SIZE);
			}



			read_len = read(fd, buff, buff_size);
			if (-1 == read_len)
			{
				fprintf(stderr, "Error %d: %s\n", errno, strerror(errno));
				free(buff);
				buff = NULL;
				break;
			}

			if (0 == read_len)
			{
				_time = time(NULL);
				tm = localtime(&_time);
				fprintf(stdout, "%d:%d:%d: Finished reading\n",
						tm->tm_hour, tm->tm_min, tm->tm_sec);
				free(buff);
				buff = NULL;
				return 0;
				break;
			}

			total_read += read_len;
			_time = time(NULL);
			tm = localtime(&_time);
			fprintf(stdout, "%d:%d:%d read %ld bytes(%ldMB), total_read %ld bytes (%ldMB)\n",
					tm->tm_hour, tm->tm_min, tm->tm_sec,
				read_len, read_len / 1024 / 1024, total_read, total_read / 1024 / 1024);

			for (i = 0; i < read_len - 1; i++)
				buff[i] = buff[i + 1];

			_time = time(NULL);
			tm = localtime(&_time);
			fprintf(stdout, "%d:%d:%d moved %ld bytes\n",
					tm->tm_hour, tm->tm_min, tm->tm_sec,
					read_len - 1);

			buff_size <<= 1;
			buff_size = buff_size > max_buff_size ? max_buff_size : buff_size;
			sleep(3);
		}

		if (buff != NULL)
			free(buff);

		close(fd);
		sleep(5);
	}
	return 0;
}
