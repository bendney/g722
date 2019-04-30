#ifndef LOGGER_H_
#define LOGGER_H_

#include <sys/time.h>
#include <time.h>
#include <syslog.h>

typedef enum log_level_flag {
    DBG_CLOSE = 0x0,
    DBG_ERR   = 0x01,
    DBG_INFO  = 0x02,
    DBG_EVENT = 0x04,
    DBG_TRACE = 0x08,
    DBG_DEBUG = 0x10,
    DBG_ALL  = DBG_ERR|DBG_INFO|DBG_TRACE|DBG_DEBUG|DBG_EVENT
} YFS_LOG_LEVEL_E;

/*��־��������ַ�ʽ:stdout or �ļ�*/
typedef enum log_record_flag {
    RECORD_STD, /*���뵽stdout*/
    RECORD_FILE  /*������ļ�*/
} YFS_LOG_RECORD_E;


#define log_fatal(fmt, arg...) do {\
            fprintf(stdout, "[fatal](%-12s %-3d) "fmt"\r\n", __FILE__, __LINE__, ##arg);\
}while(0)

#define log_error(fmt, arg...) do {\
            fprintf(stdout, "[error](%-12s %-4d) "fmt"\r\n", __FILE__, __LINE__, ##arg);\
}while(0)

#define log_info(fmt, arg...) do {\
            fprintf(stdout, "[info] (%-12s %-4d) "fmt"\r\n", __FILE__, __LINE__, ##arg);\
}while(0)

#define log_event(fmt, arg...) do {\
            fprintf(stdout, "[event](%-12s %-4d) "fmt"\r\n", __FILE__, __LINE__, ##arg);\
}while(0)

#define log_trace(fmt, arg...) do {\
            fprintf(stdout, "[trace](%-12s %-4d) "fmt"\r\n", __FILE__, __LINE__, ##arg);\
}while(0)

#define log_debug(fmt, arg...) do {\
            fprintf(stdout, "[debug](%-12s %-4d) "fmt"\r\n", __FILE__, __LINE__, ##arg);\
}while(0)

#endif
