#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <bits/getopt_ext.h>
#include <mutex>
#include <string>

using namespace std;

// 日志等级
enum {
	DEBUG = 0,
	ERROR,
	NONE
};

// 定义日志等级名称
static const char *s_level_name[NONE + 1] = {
	"DEBUG",
	"ERROR",
	"NONE"
};

#define OK 0
#define NOT_OK -1

#define LOCK_FILE_PATH "/tmp/watcher-cpp.lck"
#define MOD (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)

#define VAL_TM 10

#define VERSION "1.0.0"

// 设置日志文件大小
#define LOGGER_MAX_FILE_SIZE  (10 * 1024 * 1024)

// 定义日志文件
#define LOGGER_FILENAME "watcher.log"

// 定义日志输入函数
#define Debug(fmt, args...)                                                      \
	do                                                                       \
        {                                                                        \
		log_write(DEBUG, __FILE__, __FUNCTION__, __LINE__, fmt, ##args); \
	} while (0)

#define Error(fmt, args...)                                                      \
	do                                                                       \
        {                                                                        \
		log_write(ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##args); \
	} while (0)

mutex mutx;

// 获取父目录路径
const char *get_parent_dir() {
	static char buffer[1024];
	static bool b_first = true;
	int count;
	char *ptr = NULL;

	if(b_first) {
		lock_guard<mutex> lg(mutx);
		if(b_first) {
			count = readlink("/proc/self/exe", buffer, sizeof(buffer));
			if(count < 0 || count >= (int)sizeof(buffer)) {
				return NULL;
			}
			buffer[count] = '\0';
			ptr = strrchr(buffer, '/');
			if(ptr) {
				*ptr = 0;
				ptr =strrchr(buffer, '/');
				if(ptr) {
					*ptr = 0;
				}
			}
			b_first = false;
		}
	}
	return buffer;
}

// 日志文件大小
int log_file_size(FILE *p_fd) {
	if(ftell(p_fd) > LOGGER_MAX_FILE_SIZE) {
		if(ftruncate(fileno(p_fd), 0) != 0) {
			return NOT_OK;
		}
		rewind(p_fd);
	}
	return NOT_OK;
}

// 获取日志等级
const char * get_log_level(int level) {
	if((unsigned int)level > NONE) {
		return s_level_name[DEBUG];
	}
	return s_level_name[(unsigned int)level];
}

// 写入日志
int log_write(int level, const char *file, const char *func, int line, const char *fmt, ...) {
	int iret = 0;
	FILE *p_fd;
	char s_path[1024];
	char s_date[100];
	char s_time[100];

	struct tm *p_time;
	struct timeval current_tv;
	time_t t_now;
	va_list ap;

	if(level == NONE) {
		fprintf(stderr, "level is NONE\n");
		return OK;
	}

	if(!file || !func) {
		fprintf(stderr, "paramter error\n");
		return NOT_OK;
	}

	memset(s_path, 0x00, sizeof(s_path));
	snprintf(s_path, sizeof(s_path) - 1, "%s/log/%s", get_parent_dir(), LOGGER_FILENAME);

	if(NULL == (p_fd = fopen(s_path, "a"))) {
		fprintf(stderr, "fopen error %d\n", errno);
		return NOT_OK;
	}

	gettimeofday(&current_tv, NULL);

	t_now = current_tv.tv_sec;
	p_time = localtime(&t_now);

	sprintf(s_date, "%04d-%02d-%02d", 1900 + p_time->tm_year, p_time->tm_mon + 1, p_time->tm_mday);
	sprintf(s_time, "%02d:%02d:%02d:%ld", p_time->tm_hour, p_time->tm_min, p_time->tm_sec, current_tv.tv_usec);

	fprintf(p_fd, "(p%d) [%s %s] [%s] [%s:%d] [%s]\n\t", getpid(), s_date, s_time, get_log_level(level), file, line, func);
	va_start(ap, fmt);
	vfprintf(p_fd, fmt, ap);
	va_end(ap);
	
	fprintf(p_fd, "\n");
	if(OK != (iret = log_file_size(p_fd))) {
		fclose(p_fd);
		return iret;
	}

	fclose(p_fd);

	return iret;
}

// 设置程序为后台进程
void start_deamon() {
	signal(SIGQUIT, SIG_IGN);
	signal(SIGKILL, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGPIPE, SIG_DFL);
	signal(SIGALRM, SIG_DFL);
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);

	daemon(1, 128);
}

// 获取监控进程
int get_watcher_processer(const char *p_process) {
	char s_cmd[256];
	char s_buffer[256];
	FILE *p_fd = NULL;

	memset(s_cmd, 0x00, sizeof(s_cmd));
	snprintf(s_cmd, sizeof(s_cmd) - 1, "ps -ef|grep -v -w grep|grep -v -w gdb|grep -v -w vi|grep \"%s\"|awk '{print $8}' |head -n1|tr -d '\n'", p_process);

	if(NULL == (p_fd = popen(s_cmd, "r"))) {
		return NOT_OK;
	}

	memset(s_buffer, 0x00, sizeof(s_buffer));
	fgets(s_buffer, sizeof(s_buffer) - 1, p_fd);
	pclose(p_fd);

	if(strstr(s_buffer, p_process)) {
		return OK;
	}

	return NOT_OK;
}

// 设置lock文件
int lock_file(int fd) {
	struct flock f1;

	f1.l_type = F_WRLCK;
	f1.l_start = 0;
	f1.l_whence = SEEK_SET;
	f1.l_len = 0;

	return (fcntl(fd, F_SETLK, &f1));
}

// 判断程序是否运行
int is_running(const char *pid_file) {
	int fd;
	char buffer[16];

	fd = open(pid_file, O_RDWR | O_CREAT, MOD);
	if(fd < 0) {
		Error("Can't open %s:%s\n", pid_file, strerror(errno));
		return NOT_OK;
	}

	if(lock_file(fd) < 0) {
		if(errno == EACCES || errno == EAGAIN) {
			close(fd);
			return OK;
		}
		Error("Can't lock %s:%s\n", pid_file, strerror(errno));
		return NOT_OK;
	}
	ftruncate(fd, 0);
	sprintf(buffer, "%ld", (long)getpid());
	write(fd, buffer, strlen(buffer) + 1);
	return OK;
}

int main(int argc, char *argv[]) {
	int opt = 0;
	int option_index = 0;
	int iret = -1;
	//int i_len = 0;
	int app_last_start_time = time(nullptr);

	//char s_tmp[1024];
	//char *cmd_options = "p:n:vh";
	char *path, *name;

	static struct option long_options[] = {
		{"path", 1, 0, '-'},
		{"name", 1, 0, '-'},
		{"version", 0, 0, '-'},
		{"help", 0, 0, '-'},
		{"0, 0, 0, 0"}
	};

	while(1) {
		opt = getopt_long(argc, argv, "p:n:vh", long_options, &option_index);
		if(opt == EOF)
			break;

		switch(opt) {
			case '-':{
					 switch(option_index) {
						 case 0:{
								if(optarg) {
									path = strdup(optarg);
									fprintf(stderr, "--path %s\n", path);
								} else {
									fprintf(stderr, "--path not paramter\n");
									return -1;
								}
						}
						 case 1:{
								if(optarg) {
									name = strdup(optarg);
									fprintf(stderr, "--name %s\n", name);
								} else {
									fprintf(stderr, "--name not paramter\n");
									return -1;
								}
						}
						 case 2:{
								fprintf(stderr, "version %s\n", VERSION);
								return 0;
						}
						 case 3: {
								 fprintf(stderr, "Usage %s [p:n:vh] [par]\n"
										           "--path, -p\n program path\n"
											   "--name, -n\n program name\n"
											   "--version, -v\n version information\n"
											   "--help, -h\n help \n", argv[0]);
								 return 0;
						}
						 default:
							 fprintf(stderr, "error option_index = %d\n", option_index);
					 }
			}
			case 'p':
				 if(optarg) {
					 path = strdup(optarg);
					 fprintf(stderr, "-p %s\n", path);
				 } else {
					 fprintf(stderr, "-p not paramter\n");
					 return -1;
				 }
				 break;
			case 'n':
				 if(optarg) {
					 name = strdup(optarg);
					 fprintf(stderr, "-n %s\n", name);
				 } else {
					 fprintf(stderr, "-n not paramter\n");
					 return -1;
				 }
				 break;
			case 'v':
				 fprintf(stderr, "version %s\n", VERSION);
				 return 0;
			case 'h':
				 fprintf(stderr, "Usage %s [p:n:vh] [par]\n"
							   "--path, -p\n program path\n"
							   "--name, -n\n program name\n"
							   "--version, -v\n version information\n"
							   "--help, -h\n help \n", argv[0]);
				 return 0;
			default:
				 fprintf(stderr, "getopt returned character code 0 %02x\n", opt);
		}
	}

	if(optind < argc) {
		fprintf(stderr, "non-option argv-elements:");
		while(optind)
			printf("%s", argv[optind++]);
		printf("\n");
	}

	if(NULL == path && NULL == name) {
		fprintf(stderr, "please input --path and --name\n");
		return -1;
	}


	start_deamon();

	if(is_running(LOCK_FILE_PATH)) {
		Error("the program had started!\n");
		fprintf(stdout, "the program had started!\n this watcher program shutdow!\n");
		return NOT_OK;
	}

	while(1) {
		if(time(nullptr) - app_last_start_time > VAL_TM && (iret = get_watcher_processer(name))) {
			Error("%s don't found!\n");
			system(path);
			app_last_start_time = time(nullptr);
		}

		sleep(1);
	}

	return 0;
}
