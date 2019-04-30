#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <event.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <evhttp.h>

#include <json-c/json.h>

#include "semantic_analyzer.h"


static sa_set_para_t inParams;
static sa_get_para_t outParams;
semantic_request_t request;
semantic_answer_t answer;

static dialog_score = 0;

char databasePath[128] = "/usr/src/mms/Man-machine-switching/bin/dyxgx170627";

//YFS_LOG_LEVEL_E g_debug = DBG_ALL;
//YFS_LOG_RECORD_E g_record;

/*
int yfs_is_ars_busy(void)
{
    return 0;
}

int yfs_asr_tts_text(const char *text, char * out_file)
{
	return 0;
}
*/

custom_info_t cst_info = {
		"赵子龙",
		"先生",
};


//处理模块
void httpd_handler(struct evhttp_request *req, void *arg)
{
	char output[2048] = "\0";
	char tmp[1024];
	const char *uri;
	char *decoded_uri;
	char *post_data = NULL;

	int fd = -1;
	char audioPath[128];
	struct stat statbuf;

	struct evkeyvalq params;
	struct evbuffer * buf;
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;

	char * tmp_value = NULL;
	const char * headerContent = NULL;

	json_object *my_object = NULL;
    json_object *json_obj = NULL;
    json_object *value_obj= NULL;

	my_object = json_object_new_object();
	buf = evbuffer_new();

	switch (evhttp_request_get_command(req)) {
		case EVHTTP_REQ_GET: cmdtype = "GET"; break;
		case EVHTTP_REQ_POST: cmdtype = "POST"; break;
		case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
		case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
		case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
		case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
		case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
		case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
		case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
		default: cmdtype = "unknown"; break;
	}

	printf("Received a %s request for %s\n",
			cmdtype, evhttp_request_get_uri(req));

	if (strcmp(cmdtype, "GET") == 0) {
		uri = evhttp_request_uri(req);
		sprintf(tmp, "evhttp_request_uri=%s\n", uri);
		strcat(output, tmp);

		sprintf(tmp, "req->uri=%s\n", req->uri);
		strcat(output, tmp);

		//decoded uri
		decoded_uri = evhttp_decode_uri(uri);
		sprintf(tmp, "decoded_uri=%s\n", decoded_uri);
		strcat(output, tmp);

		//解析URI的参数(即GET方法的参数)
		evhttp_parse_query(decoded_uri, &params);

		sprintf(tmp, "q=%s\n", evhttp_find_header(&params, "q"));
		strcat(output, tmp);
		sprintf(tmp, "s=%s\n", evhttp_find_header(&params, "s"));
		strcat(output, tmp);
		free(decoded_uri);

		printf("%s\n", output);
	} else if (strcmp(cmdtype, "DELETE") == 0) {
		outParams.hg_status = 0;
		get_semantic_analyzer(GET_HANGUP_STATUS, &outParams);
		dialog_score = get_semantic_analyzer(GET_LEVEL, &outParams);

		json_object_object_add(my_object, "code", json_object_new_int(0));
		json_object_object_add(my_object, "level", json_object_new_string(&outParams.level));
		json_object_object_add(my_object, "hgStatus", json_object_new_int(outParams.hg_status));

		evhttp_add_header(req->output_headers, "Content-Type", json_object_to_json_string(my_object));

		evbuffer_add(buf, "Data is null", 12);
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);

		json_object_put(my_object);
		return;
	} else if (strcmp(cmdtype, "POST") == 0) {
		post_data = (char *) EVBUFFER_DATA(req->input_buffer);
		if (post_data == NULL) {
			evhttp_add_header(req->output_headers, "Content-Type", "{audio:1}");

			evbuffer_add(buf, "Data is null", 12);
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			return;
		}
		json_obj = json_tokener_parse(post_data);
		if (json_obj == NULL) {
			evhttp_add_header(req->output_headers, "Content-Type", "{audio:1}");

			evbuffer_add(buf, "Data is null", 12);
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);

			json_object_put(my_object);
			return;
		}
		printf("Parsed json: %s\n", json_object_to_json_string(json_obj));

		if (json_object_object_get_ex(json_obj, "tempNum", &value_obj)) {
			tmp_value = (char *)json_object_get_string(value_obj);

			inParams.db_path = databasePath;
			set_semantic_analyzer(SET_DB_PATH, &inParams);
			inParams.custom_info = cst_info;
			set_semantic_analyzer(SET_CUSTOM_INFO, &inParams);
			strcpy(inParams.ai_version, "1.2.2");
			set_semantic_analyzer(SET_AI_VERSION, &inParams);

			memset(request.word, 0x0, sizeof(request.word));
			request.type = WITH_CONNECT;
		} else if (json_object_object_get_ex(json_obj, "message", &value_obj)) {
			tmp_value = (char *)json_object_get_string(value_obj);
			memset(request.word, 0x0, sizeof(request.word));
			strcpy(request.word, tmp_value);
			request.type = WITH_SPEECH;
			gettimeofday(&request.start, NULL);
			gettimeofday(&request.end, NULL);
		} else if (json_object_object_get_ex(json_obj, "audioEvent", &value_obj)) {
			tmp_value = (char *)json_object_get_string(value_obj);
			/* Play start and update context */
			if (strcmp(tmp_value, "begin") == 0) {
				inParams.context = answer.context;
				set_semantic_analyzer(SET_UP_CONTEXT, &inParams);
			}
			/* Play end and set status to dialog */
			if (strcmp(tmp_value, "end") == 0) {
				set_semantic_analyzer(SET_PLAY_END_STATUS, NULL);
			}

			evhttp_add_header(req->output_headers, "Content-Type", "{audio:1}");

			json_object_object_add(my_object, "code", json_object_new_int(0));
			evbuffer_add(buf, json_object_to_json_string(my_object), strlen(json_object_to_json_string(my_object)));

			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);

			json_object_put(my_object);
			return;
		}
	}

	semantic_analyzer(&request, &answer);
	if (answer.type == CLOSE_ANSWER) {
		json_object_object_add(my_object, "response", json_object_new_string(answer.word));
		json_object_object_add(my_object, "type", json_object_new_string("CLOSE_ANSWER"));
		json_object_object_add(my_object, "record", json_object_new_string(answer.context.record));
	} else if (answer.type == READ_ANSWER) {
		json_object_object_add(my_object, "response", json_object_new_string(answer.word));
		json_object_object_add(my_object, "type", json_object_new_string("READ_ANSWER"));
		json_object_object_add(my_object, "record", json_object_new_string(answer.context.record));
		json_object_object_add(my_object, "dlgId", json_object_new_string("15320513420000008"));
	} else if (answer.type == CLEAN_ANSWER) {
		json_object_object_add(my_object, "response", json_object_new_string(answer.word));
		json_object_object_add(my_object, "type", json_object_new_string("CLEAN_ANSWER"));
		json_object_object_add(my_object, "record", json_object_new_string(answer.context.record));
		json_object_object_add(my_object, "dlgId", json_object_new_string("15320513420000008"));
	} else if (answer.type == SKIP_IVR_ANSWER) {
		json_object_object_add(my_object, "response", json_object_new_string(answer.word));
		json_object_object_add(my_object, "type", json_object_new_string("SKIP_IVR_ANSWER"));
		json_object_object_add(my_object, "record", json_object_new_string(answer.context.record));
		json_object_object_add(my_object, "dlgId", json_object_new_string("15320513420000008"));
	} else if (answer.type == KEEP_ANSWER) {
		json_object_object_add(my_object, "response", json_object_new_string(answer.word));
		json_object_object_add(my_object, "type", json_object_new_string("KEEP_ANSWER"));
		json_object_object_add(my_object, "record", json_object_new_string(answer.context.record));
	}
	headerContent = json_object_to_json_string(my_object);
	if (headerContent == NULL) {
		return;
	}
	printf("Send header Content-Type: %s\n",
		   	json_object_to_json_string(my_object));

	evhttp_add_header(req->output_headers, "Content-Type", headerContent);

	evutil_snprintf(audioPath, 128, "%s/record/%s.wav",
		   	databasePath, answer.context.record);

	if ((fd = open(audioPath, O_RDONLY)) < 0) {
		return;
	}

	if (fstat(fd, &statbuf)<0) {
		/* Make sure the length still matches, now that we
		 * opened the file :/ */
		return;
	}
	evbuffer_add_file(buf, fd, 0, statbuf.st_size);

	evhttp_send_reply(req, HTTP_OK, "OK", buf);
	evbuffer_free(buf);


	json_object_put(my_object);
	
	if (json_obj) {
		json_object_put(json_obj);
	}
}

/* Callback used for the /dump URI, and for every non-GET request:
 * dumps all information to stdout and gives back a trivial 200 ok 
 */
static void dump_request_cb(struct evhttp_request *req, void *arg)
{
	const char *cmdtype;
	struct evkeyvalq *headers;
	struct evkeyval *header;
	struct evbuffer *buf;

	switch (evhttp_request_get_command(req)) {
		case EVHTTP_REQ_GET:
		   	cmdtype = "GET"; break;
		case EVHTTP_REQ_POST:
		   	cmdtype = "POST"; break;
		case EVHTTP_REQ_HEAD:
		   	cmdtype = "HEAD"; break;
		case EVHTTP_REQ_PUT:
		   	cmdtype = "PUT"; break;
		case EVHTTP_REQ_DELETE:
		   	cmdtype = "DELETE"; break;
		case EVHTTP_REQ_OPTIONS:
		   	cmdtype = "OPTIONS"; break;
		case EVHTTP_REQ_TRACE:
		   	cmdtype = "TRACE"; break;
		case EVHTTP_REQ_CONNECT:
		   	cmdtype = "CONNECT"; break;
		case EVHTTP_REQ_PATCH:
		   	cmdtype = "PATCH"; break;
		default:
		   	cmdtype = "unknown"; break;
	}

	printf("Received a %s request for %s\nHeaders:\n",
			cmdtype, evhttp_request_get_uri(req));

	headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
			header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}

	buf = evhttp_request_get_input_buffer(req);
	/*
	puts("Input data: <<<");
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128];
		n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
		if (n > 0)
			(void) fwrite(cbuf, 1, n, stdout);
	}
	puts(">>>");
	*/

	evhttp_send_reply(req, 200, "OK", NULL);
}

void show_help()
{
	char *help = "written by Min (http://54min.com)\n\n"
		"-l <ip_addr> interface to listen on, default is 0.0.0.0\n"
		"-p <num>     port number to listen on, default is 1984\n"
		"-d           run as a deamon\n"
		"-t <second>  timeout for a http request, default is 120 seconds\n"
		"-h           print this help and exit\n"
		"\n";
	fprintf(stderr, help);
}
//当向进程发出SIGTERM/SIGHUP/SIGINT/SIGQUIT的时候，终止event的事件侦听循环
void signal_handler(int sig)
{
	switch (sig) {
		case SIGTERM:
		case SIGHUP:
		case SIGQUIT:
		case SIGINT:
			event_loopbreak();  //终止侦听event_dispatch()的事件侦听循环，执行之后的代码
			break;
	}
}

int main(int argc, char *argv[])
{
	//自定义信号处理函数
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	//默认参数
	char *httpd_option_listen = "0.0.0.0";
	int httpd_option_port = 8080;
	int httpd_option_daemon = 0;
	int httpd_option_timeout = 120; //in seconds

	//获取参数
	int c;
	while ((c = getopt(argc, argv, "l:p:dt:h")) != -1) {
		switch (c) {
			case 'l' :
				httpd_option_listen = optarg;
				break;
			case 'p' :
				httpd_option_port = atoi(optarg);
				break;
			case 'd' :
				httpd_option_daemon = 1;
				break;
			case 't' :
				httpd_option_timeout = atoi(optarg);
				break;
			case 'h' :
			default :
				show_help();
				exit(EXIT_SUCCESS);
		}
	}

	//判断是否设置了-d，以daemon运行
	if (httpd_option_daemon) {
		pid_t pid;
		pid = fork();
		if (pid < 0) {
			perror("fork failed");
			exit(EXIT_FAILURE);
		}
		if (pid > 0) {
			//生成子进程成功，退出父进程
			exit(EXIT_SUCCESS);
		}
	}

	/* 使用libevent创建HTTP Server */

	//初始化event API
	event_init();

	//创建一个http server
	struct evhttp *httpd;
	httpd = evhttp_start(httpd_option_listen, httpd_option_port);
	evhttp_set_timeout(httpd, httpd_option_timeout);

	//也可以为特定的URI指定callback
	evhttp_set_cb(httpd, "/vacantNumber", dump_request_cb, NULL);

	//指定generic callback
	evhttp_set_gencb(httpd, httpd_handler, NULL);
	

	//循环处理events
	event_dispatch();

	evhttp_free(httpd);
	return 0;
}

