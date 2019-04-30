#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <esl.h>
#include <glib.h>

#include "venilla_xml.h"
#include "venilla_list.h"

#define AGENT_XML_CONFIG		"venilla.xml"
#define RECORD_DIRECTORY		"/tmp/record/"

#define AGENT_XML_MUSIC			"suite.wav"

#define TONE_PARK				"9"
#define TONE_BRIDGE				"8"


static GList* venilla_list		= NULL;
static esl_mutex_t * venilla_list_mutex;

list_node * xml_list;

struct xml_document * document = NULL;

typedef struct {
	char caller[32];
	char agent[10][16];
	unsigned long numAgents;
} AGENT_NODE;

typedef struct {
	char callee_uuid[48];
	char caller_uuid[48];
	char agent_uuid[48];
	char channel_state[16];
} CHAIN_MEMBER;

struct xml_document * xml_document_attach()
{
	FILE * xmlhandle = fopen(AGENT_XML_CONFIG, "rb");
	if (xmlhandle == NULL) {
		printf("Couldn't open %s document\n", AGENT_XML_CONFIG);
		return NULL;
	}

	struct xml_document* document = xml_open_document(xmlhandle);
	if (!document) {
		printf("Couldn't open document\n");
		return NULL;
	}

	return document;
}

void xml_document_dettach(struct xml_document * document)
{
	/* Remember to free the document or you'll risk a memory leak */
	xml_document_free(document, true);
}

void child_element_get(struct xml_document * document)
{
	struct xml_node* root = xml_document_root(document);

	/* Say Hello World :-) */
	struct xml_node* first_child = xml_node_child(root, 1);
	struct xml_string* child_name = xml_node_name(first_child);
	struct xml_string* child_content = xml_node_content(first_child);

	/* Watch out: `xml_string_copy' will not 0-terminate your buffers! (but
	 *     `calloc' will :-)
	 */
	uint8_t* child_name_0 = calloc(xml_string_length(child_name) + 1, sizeof(uint8_t));
	uint8_t* child_content_0 = calloc(xml_string_length(child_content) + 1, sizeof(uint8_t));
	xml_string_copy(child_name, child_name_0, xml_string_length(child_name));
	xml_string_copy(child_content, child_content_0, xml_string_length(child_content));

	printf("%s %s\n", child_name_0, child_content_0);
	free(child_name_0);
	free(child_content_0);
}


void xml_element_parse(struct xml_document * document, list_node * xml_list)
{
	int i;
	unsigned long j;
	struct xml_node* root_element;
	struct xml_node* root;
	char element_word[16];
	char agent_word[16];

	AGENT_NODE * node_agent = NULL;
	struct xml_node * child_node = NULL;
	struct xml_string * child_node_content = NULL; 
	i = 0;

   	root = xml_document_root(document);

	/* Extract amount of Root/This children */
	while (root_element = xml_node_child(root, i))
	{
		snprintf(element_word, 16, "Element%d", i);

		/* Create node for element content */
		node_agent = (AGENT_NODE *)malloc(sizeof(AGENT_NODE));;
		if (NULL == node_agent) {
			continue;
		}
		list_insert_after(xml_list, (void *)node_agent);

		/* Get CallerId node element value from xml */
		child_node = xml_easy_child(root, element_word, "CallerId", 0);
		if (NULL == child_node) {
			continue;
		}
		child_node_content = xml_node_content(child_node);
		if (NULL == child_node_content) {
			continue;
		}
		xml_string_copy(child_node_content,
				node_agent->caller, xml_string_length(child_node_content));

		printf("\t<CallerId>%s</CallerId>\n", node_agent->caller);
		/* reduce the caller id element */
		node_agent->numAgents = (unsigned long)xml_node_children(root_element) - 1;
		/* Get agent node element value from xml */
		for (j = 0; j <  node_agent->numAgents; j++) {
			snprintf(agent_word, 16, "Agent%ld", j);
			child_node = xml_easy_child(root, element_word, agent_word, 0);
			if (child_node == NULL) {
				break;
			}

			child_node_content = xml_node_content(child_node);
			if (child_node_content == NULL) {
				break;
			}

			xml_string_copy(child_node_content,
					node_agent->agent[j], xml_string_length(child_node_content));
			printf("\t\t<Agent%ld>%s</Agent%ld>\n", j, node_agent->agent[j], j);
		}
		i++;
	}
}

static int compare_callee(CHAIN_MEMBER * list_data, void * data)
{
	if ((list_data == NULL) && (data == NULL)) {
		return 1;
	}

	return strcmp(list_data->callee_uuid, data);
}

static int compare_caller(CHAIN_MEMBER * list_data, void * data)
{
	if ((list_data == NULL) && (data == NULL)) {
		return 1;
	}

	return strcmp(list_data->caller_uuid, data);
}

static int compare_agent(CHAIN_MEMBER * list_data, void * data)
{
	if ((list_data == NULL) && (data == NULL)) {
		return 1;
	}

	return strcmp(list_data->agent_uuid, data);
}

int search_by_caller(list_node * list, void * data)
{
	if (list->data == NULL) {
		return 0;
	}

	AGENT_NODE * list_caller_id = (AGENT_NODE *)(list->data);
	char * caller_id = (char *)data;

	if (!strcmp(list_caller_id->caller, caller_id)) {
		return 1;
	}

	return 0;
}

char * fgapi_execute(esl_handle_t * handle, const char * argv_command)
{
	if (!handle && !argv_command) {
		return "ERR";
	}

	esl_status_t status;
	char cmd_str[1024] = "";

	esl_snprintf(cmd_str, sizeof(cmd_str), "api %s\nconsole_execute: true\n\n", argv_command);
	status = esl_send_recv_timed(handle, cmd_str, 0);
	if (status != ESL_SUCCESS) {
		return "ERR";
	}

	if (handle->last_sr_event && handle->last_sr_event->body) {
		//return strtok(handle->last_sr_event->body, " ") + 1;
		return handle->last_sr_event->body;
		printf("%s\n", handle->last_sr_event->body);
	}

	return "ERR";
}

char * bgapi_execute(esl_handle_t * handle, const char * argv_command)
{
	if (!handle && !argv_command) {
		return "ERR";
	}

	esl_status_t status;
	char cmd_str[1024] = "";

	esl_snprintf(cmd_str, sizeof(cmd_str), "bgapi %s\nconsole_execute: true\n\n", argv_command);
	status = esl_send_recv_timed(handle, cmd_str, 0);
	if (status != ESL_SUCCESS) {
		return "ERR";
	}

	if (handle->last_sr_event && handle->last_sr_event->body) {
		//return strtok(handle->last_sr_event->body, " ") + 1;
		return handle->last_sr_event->body;
		printf("%s\n", handle->last_sr_event->body);
	}

	return "ERR";
}

static char * internal_reg_list(esl_handle_t * handle, AGENT_NODE * node)
{
	if (!handle && !node) {
		return NULL;
	}
	
	int j;
	char * result = NULL;

	esl_log(ESL_LOG_INFO, "====================Registration List========================\n");
	for (j = 0; j < node->numAgents; j++) {
		/* Get current registrations list, TO-DO:must be the pure number list */
		result = fgapi_execute(handle, "sofia xmlstatus profile internal reg");
		if (!strcasecmp(result, "ERR")) {
			continue;
		}
		/* This agent doesn't exsit in registrations list */
		if (strstr(result, node->agent[j]) == NULL) {
			continue;
		}

		esl_log(ESL_LOG_INFO, "Registration %s\n", node->agent[j]);
		/* Get current calls list, TO-DO:must be the pure number list */
		result = fgapi_execute(handle, "show calls");
		if (!strcasecmp(result, "ERR")) {
			continue;
		}
		/* To find current idle extension */
		if (strstr(result, node->agent[j]) == NULL) {
			break;
		}
	}

	if (node->numAgents == j) {
		return NULL;
	}

	esl_log(ESL_LOG_INFO, "====================Registration List========================\n");

	return node->agent[j];
}

char * channel_record_start(esl_handle_t * handle, const char * uuid)
{
	char * result;
	char command[1024] = "";
	GList * seris_uuid = NULL;
	CHAIN_MEMBER * member = NULL;

	esl_mutex_lock(venilla_list_mutex);
	seris_uuid = g_list_find_custom(venilla_list,
			(gpointer)uuid, (GCompareFunc)compare_callee);
	esl_mutex_unlock(venilla_list_mutex);
	if (seris_uuid == NULL) {
		return NULL;
	}

	member = (CHAIN_MEMBER *)(seris_uuid->data);
	if (member->agent_uuid == NULL) {
		return NULL;
	}

	esl_snprintf(command, sizeof(command),
			"uuid_record %s start /tmp/record/%s.mp3",
			member->callee_uuid, member->callee_uuid);
	result = fgapi_execute(handle, command);

	return result;
}

char * channel_record_stop(esl_handle_t * handle, const char * uuid)
{
	char * result;
	char command[1024] = "";
	GList * seris_uuid = NULL;
	CHAIN_MEMBER * member = NULL;

	esl_mutex_lock(venilla_list_mutex);
	seris_uuid = g_list_find_custom(venilla_list,
			(gpointer)uuid, (GCompareFunc)compare_callee);
	esl_mutex_unlock(venilla_list_mutex);
	if (seris_uuid == NULL) {
		return NULL;
	}

	member = (CHAIN_MEMBER *)(seris_uuid->data);
	if (member->agent_uuid == NULL) {
		return NULL;
	}

	esl_snprintf(command, sizeof(command),
			"uuid_record %s stop /tmp/record/%s.mp3",
			member->callee_uuid, member->callee_uuid);
	result = fgapi_execute(handle, command);

	return result;
}

static void event_control(esl_handle_t *handle, esl_event_t *event)
{
	char command[1024] = "";
	const char * result;
	CHAIN_MEMBER * member = NULL;
	GList * seris_uuid = NULL;

	esl_log(ESL_LOG_INFO, "Event Socket Library event id %d\n", event->event_id);

	switch (event->event_id) {
		case ESL_EVENT_CHANNEL_CREATE:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			const char * channel_uuid = esl_event_get_header(event, "Channel-Call-UUID");
			const char * direction = esl_event_get_header(event, "Call-Direction");
			esl_log(ESL_LOG_INFO, "Channel %s create direction %s\n", uuid, direction);
#if 0
			/* In case receive park operation create channel and inbound channel */
			if (!strcasecmp(uuid, channel_uuid)
				|| !strcasecmp(direction, "inbound")) {
				break;
			}

			/* Create new call member WARNING: memory leak when park channel create :has been Fixed */
			member = (CHAIN_MEMBER *)malloc(sizeof(CHAIN_MEMBER));;
			if (NULL == member) {
				break;
			}
			memset(member, 0, sizeof(CHAIN_MEMBER));

			strncpy(member->caller_uuid, channel_uuid, 48);
			strncpy(member->callee_uuid, uuid, 48);

			esl_mutex_lock(venilla_list_mutex);
			venilla_list = g_list_append(venilla_list, member);
			esl_mutex_unlock(venilla_list_mutex);

			esl_log(ESL_LOG_INFO, "Channel %s create\n", member->callee_uuid);
#endif

			break;
		}
		case ESL_EVENT_CHANNEL_DESTROY:
		{
			char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s destory\n", uuid);
#if 0

			/* Destory the call chain space, find member by channel uuid */
			esl_mutex_lock(venilla_list_mutex);
			seris_uuid = g_list_find_custom(venilla_list, (gpointer)uuid, (GCompareFunc)compare_callee);
			esl_mutex_unlock(venilla_list_mutex);
			if (seris_uuid == NULL) {
				break;
			}

			/* Hangup parking agent channel */
			member = (CHAIN_MEMBER *)(seris_uuid->data);
			if (member->agent_uuid == NULL) {
				break;
			}

			esl_snprintf(command, sizeof(command), "uuid_exists %s ", member->agent_uuid);
			result = fgapi_execute(handle, command);
			if (!strcasecmp(result, "true")) {
				esl_snprintf(command, sizeof(command), "uuid_kill %s ", member->agent_uuid);
				fgapi_execute(handle, command);
			}

			esl_log(ESL_LOG_INFO, "Channel %s destroy\n", member->callee_uuid);

			esl_mutex_lock(venilla_list_mutex);
			venilla_list = g_list_remove(venilla_list, member);
			esl_mutex_unlock(venilla_list_mutex);

			if (member != NULL) {
				free(member);
			}
			member = NULL;
#endif

			break;
		}
		case ESL_EVENT_CHANNEL_ANSWER:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");

			/*
			esl_log(ESL_LOG_INFO, "Channel %s record start\n", uuid);

			result = channel_record_start(handle, uuid);
			if (result == NULL) {
				break;
			}
			*/
			esl_log(ESL_LOG_INFO, "Channel %s answer\n", uuid);

			break;
		}
		case ESL_EVENT_CHANNEL_HANGUP:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");

			/*
			esl_log(ESL_LOG_INFO, "Channel %s record stop\n", uuid);

			result = channel_record_stop(handle, uuid);
			if (result == NULL) {
				break;
			}
			*/
			esl_log(ESL_LOG_INFO, "Channel %s hangup\n", uuid);

			break;
		}
		case ESL_EVENT_CHANNEL_CALLSTATE:
		{
			char * uuid = esl_event_get_header(event, "Unique-ID");
			const char * state = esl_event_get_header(event, "Channel-Call-State");

			esl_log(ESL_LOG_INFO, "Channel %s state %s\n", uuid, state);

#if 0
			/* Index the call_chain array, get parked agent uuid */
			esl_mutex_lock(venilla_list_mutex);
			seris_uuid = g_list_find_custom(venilla_list, (gpointer)uuid, (GCompareFunc)compare_agent);
			esl_mutex_unlock(venilla_list_mutex);
			if (seris_uuid == NULL) {
				break;
			}

			member = (CHAIN_MEMBER *)(seris_uuid->data);
			strncpy(member->channel_state, state, 16);
#endif

			break;
		}
		case ESL_EVENT_CHANNEL_PARK:
		{
			char * uuid = esl_event_get_header(event, "Unique-ID");
			const char * callerId = esl_event_get_header(event, "Caller-Caller-ID-Number");
#if 0
			/* Index the call_chain array, get parked agent uuid */
			esl_mutex_lock(venilla_list_mutex);
			seris_uuid = g_list_find_custom(venilla_list, (gpointer)uuid, (GCompareFunc)compare_agent);
			esl_mutex_unlock(venilla_list_mutex);
			if (seris_uuid == NULL) {
				break;
			}

			esl_log(ESL_LOG_INFO, "Channel %s park\n", uuid);

			member = (CHAIN_MEMBER *)(seris_uuid->data);

			/* send dtmf digits for parked response */
			esl_snprintf(command, sizeof(command),
					"uuid_send_dtmf %s #%s", member->caller_uuid, callerId);
			fgapi_execute(handle, command);

			esl_snprintf(command, sizeof(command),
					"uuid_broadcast %s %s", member->agent_uuid, AGENT_XML_MUSIC);
			bgapi_execute(handle, command);

			strncpy(member->channel_state, "PARKING", 16);
#endif

			break;
		}
		case ESL_EVENT_CHANNEL_EXECUTE:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s execute\n", uuid);

			break;
		}

		case ESL_EVENT_CHANNEL_PROGRESS:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s progress\n", uuid);

			break;
		}
		case ESL_EVENT_CHANNEL_PROGRESS_MEDIA:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s progress media\n", uuid);

			break;
		}
		case ESL_EVENT_CHANNEL_UUID:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s uuid\n", uuid);

			break;
		}
		case ESL_EVENT_CHANNEL_OUTGOING:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s outgoing\n", uuid);

			break;
		}
		case ESL_EVENT_HEARTBEAT:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s HEARTBEAT\n", uuid);

			break;
		}
		case ESL_EVENT_CUSTOM:
		{
			const char * uuid = esl_event_get_header(event, "Unique-ID");
			esl_log(ESL_LOG_INFO, "Channel %s CUSTOM\n", uuid);

			break;
		}

#if 0
	ESL_EVENT_CHANNEL_HANGUP,
	ESL_EVENT_CHANNEL_HANGUP_COMPLETE,
	ESL_EVENT_CHANNEL_EXECUTE,
	ESL_EVENT_CHANNEL_EXECUTE_COMPLETE,
	ESL_EVENT_CHANNEL_HOLD,
	ESL_EVENT_CHANNEL_UNHOLD,
	ESL_EVENT_CHANNEL_BRIDGE,
	ESL_EVENT_CHANNEL_UNBRIDGE,
	ESL_EVENT_CHANNEL_PROGRESS,
	ESL_EVENT_CHANNEL_PROGRESS_MEDIA,
	ESL_EVENT_CHANNEL_OUTGOING,
	ESL_EVENT_CHANNEL_PARK,
	ESL_EVENT_CHANNEL_UNPARK,
	ESL_EVENT_CHANNEL_APPLICATION,
	ESL_EVENT_CHANNEL_ORIGINATE,
	ESL_EVENT_CHANNEL_UUID,
#endif
		default:
			break;
	}

}

static void _sleep_ns(int secs, long nsecs) {
#ifndef WIN32
	if (nsecs > 999999999) {
		secs += nsecs/1000000000;
		nsecs = nsecs % 1000000000;
	}
	{
		struct timespec ts = { secs, nsecs };
		nanosleep(&ts, NULL);
	}
#else
	Sleep(secs*1000 + nsecs/1000000);
#endif
}

static void sleep_s(int secs) { _sleep_ns(secs, 0); }

int main(void)
{
	esl_handle_t handle = {{0}};

	esl_global_set_default_logger(ESL_LOG_LEVEL_INFO);

	while (esl_connect(&handle, "127.0.0.1", 8021, NULL, "ClueCon") != ESL_SUCCESS) {
		sleep_s(1);
		esl_log(ESL_LOG_ERROR, "Error connecting FreeSWITCH, Retrying \n");
		continue;
	}

	esl_log(ESL_LOG_INFO, "Connected to FreeSWITCH\n");
	esl_events(&handle, ESL_EVENT_TYPE_PLAIN, "CHANNEL_CREATE CHANNEL_DESTROY CHANNEL_ANSWER CHANNEL_HANGUP CHANNEL_CALLSTATE CHANNEL_PARK DTMF CUSTOM robot::report");

	esl_log(ESL_LOG_INFO, "%s\n", handle.last_sr_reply);

#if 0
	esl_mutex_create(&venilla_list_mutex);

	xml_list = list_create(NULL);

	document = xml_document_attach();

	xml_element_parse(document, xml_list);
#endif

	handle.event_lock = 1;
	while ((esl_recv_event(&handle, 1, NULL)) == ESL_SUCCESS) {
		if (handle.last_ievent) {
			event_control(&handle, handle.last_ievent);
		}
	}

#if 0
	list_destroy(&xml_list);

	xml_document_dettach(document);

	esl_mutex_destroy(&venilla_list_mutex);
#endif

	esl_disconnect(&handle);
	
	return 0;
}

