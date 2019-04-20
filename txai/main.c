#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <json-c/json.h>

#include <openssl/md5.h>

#include "url.h"


#define  AAITTS_URI			"https://api.ai.qq.com/fcgi-bin/aai/aai_tts"

typedef struct memory_struct_tag {
	char *memory;
	size_t size;
} JSOM_MEM_S;


static size_t writefunc(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
    JSOM_MEM_S *mem = (JSOM_MEM_S *)userp;
    char *tmp = NULL;
	
    tmp = (char *)realloc(mem->memory, mem->size + realsize + 1);
    if(tmp == NULL) {
        /* out of memory! */
        return 0;
    }
    else {
        mem->memory = tmp;
    }
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
	
    return realsize;
}


void uppercase(char * temp)
{
	char * name;
	name = strtok(temp,":");

	// Convert to upper case
	char *s = name;
	while (*s) {
		*s = toupper((unsigned char) *s);
		s++;
	}

}


/*
 *
 */
static int GenerateSign(const char * params, int paramSize, const char * app_key, char * sign)
{
	unsigned char result[MD5_DIGEST_LENGTH];
	char * params_encoded = NULL;
	int new_length;
	int value = 0;
	unsigned i = 0;

	if ((params == NULL) || (paramSize == 0) || (app_key == NULL)) {
		return 0x11;
	}

	params_encoded = php_url_encode(params, paramSize, &new_length);
	if (params_encoded == NULL) {
		return 0x12;
	}

	value = sprintf(params_encoded, "%s&app_key=%s", params_encoded, app_key);
	if (value <= 0) {
		return 0x13;
	}

	printf("Key: %s encoded: %s \n", app_key, params_encoded);

	MD5(params_encoded, strlen(params_encoded), result);

	for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
		sprintf(sign, "%s%02x", sign, result[i]);
	}
	uppercase(sign);

	printf("Signature %s\n", sign);

	return 0;
}


static int GenerateStr(char * str, int length)
{
	int i, flag;

	srand(time(NULL));//通过时间函数设置随机数种子，使得每次运行结果随机。

	for (i = 0; i < length; i ++) {
		flag = rand()%3;
		switch(flag)
		{
			case 0:
				str[i] = rand()%26 + 'a'; 
				break;
			case 1:
				str[i] = rand()%26 + 'A'; 
				break;
			case 2:
				str[i] = rand()%10 + '0'; 
				break;
		}
	}
	printf("%s\n", str);//输出生成的随机数。

	return 0;
}


const char * jsonParseValue(const char * data, const char *key)
{
  const char * value = NULL;
  struct json_object *json_obj = NULL;
  struct json_object *json_string = NULL;

  if ((data == NULL) || (key == NULL)) {
  	return NULL;
  }

  json_obj = json_tokener_parse(data);
  if (json_obj == NULL) {
  	return NULL;
  }

  json_object_object_get_ex(json_obj, key, &json_string);
  value = json_object_get_string(json_string);
  if (value == NULL) {
  	return NULL;
  }

  return value;
}


int AaiTts_ForText(char * text, char * outFilePath)
{
  CURL *curl;
  CURLcode res;

  char nonce_str[24] =  {'\0'};
  char params[512] = {0};
  char app_id[32] = "2107987143";
  char app_key[32] = "74NMXblr3dM1nRyZ";
  const char * speechString = NULL;
  const char * dataString = NULL;
  unsigned char * base64DecodeOutput = NULL;
  int file_length;
  int paramSize = 0;
  FILE * fp = NULL;
  char signature[32] = {0};

  JSOM_MEM_S curlDataBuffer = {0,0};

  if ((text == NULL) || (outFilePath == NULL)) {
	  return 0;
  }

  GenerateStr(nonce_str, 16);
  //printf("text %s nonce_str: %s\n", text, nonce_str);

  sprintf(params, 
		  "aht=0&apc=58&app_id=%s&format=3&nonce_str=%s&speaker=5&speed=100&text=%s&time_stamp=%ld&volume=0",
		  app_id, nonce_str, text, time(NULL));	

  //printf("params size %d App key %s\n", paramSize, app_key);
  paramSize = strlen(params);
  GenerateSign(params, paramSize, app_key, signature);
  sprintf(params, "%s&sign=%s", params, signature);

  printf("%s\n", params);

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if (!curl) {
	curl_global_cleanup();
	return 0;
  }

  curl_easy_setopt(curl, CURLOPT_URL, AAITTS_URI);

  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlDataBuffer);

  /* Now specify the POST data */
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params);

  /* Perform the request, res will get the return code */
  res = curl_easy_perform(curl);
  /* Check for errors */
  if (res != CURLE_OK)
	  fprintf(stderr, "curl_easy_perform() failed: %s\n",
			  curl_easy_strerror(res));

  dataString = jsonParseValue(curlDataBuffer.memory, "data");
  if (dataString == NULL) {
  	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0; 
  }

  speechString = jsonParseValue(dataString, "speech");
  if (speechString == NULL) {
  	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0; 
  }

  base64DecodeOutput = base64_decode(speechString,
		  strlen(speechString), &file_length);
  if (base64DecodeOutput == NULL) {
  	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0; 
  }


  fp = fopen(outFilePath, "wb");
  if (fp == NULL) {
	  return 0;
  }

  fwrite(base64DecodeOutput, file_length, 1, fp);

  fclose(fp);

  free(base64DecodeOutput);
  /* always cleanup */
  curl_easy_cleanup(curl);

  curl_global_cleanup();

  return 0;
}

int main()
{
  char text[] = "你好我这里是声讯网络科技有限公司";
  AaiTts_ForText(text, "/tmp/test.mp3");

  char text1[] = "你好我这里是声讯网络科技有";
  AaiTts_ForText(text1, "/tmp/tt.mp3");
  return 0;
}
