package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"
	"log"
	"mime"
	"net/http"
	"strconv"
	"strings"
	"github.com/aliyun/alibaba-cloud-sdk-go/sdk"
	"github.com/aliyun/alibaba-cloud-sdk-go/sdk/requests"
)

type Content struct {
	Code             string `json:"code"`
	IsVacantNumber   int    `json:"isVacantNumber"`
	HangupState      string `json:"hangup_state"`
	VacantNumberType string `json:"vacantNumberType"`
}

type Result struct {
	NlsRequestId string `json:"NlsRequestId"`
	RequestId    string `json:"RequestId"`
	Token		 TokenSub  `json:"Token"`
}

type TokenSub struct {
	ExpireTime int64
	Id		   string
	UserId	   string
}

const appkey string = "4O96nIItC1kCGSt2"
const token string = "4O96nIItC1kCGSt2"

func OriginateProcess(appkey string, token string, audio []byte, format string, sampleRate int,
	enablePunctuationPrediction bool, enableInverseTextNormalization bool, enableVoiceDetection bool) (string, error) {
	var url string = "http://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/asr"
	url = url + "?appkey=" + appkey
	url = url + "&format=" + format
	url = url + "&sample_rate=" + strconv.Itoa(sampleRate)
	if enablePunctuationPrediction {
		url = url + "&enable_punctuation_prediction=" + "true"
	}
	if enableInverseTextNormalization {
		url = url + "&enable_inverse_text_normalization=" + "true"
	}
	if enableVoiceDetection {
		url = url + "&enable_voice_detection=" + "false"
	}
	request, err := http.NewRequest("POST", url, bytes.NewBuffer(audio))
	if err != nil {
		panic(err)
	}
	request.Header.Add("X-NLS-Token", token)
	request.Header.Add("Content-Type", "application/octet-stream")

	client := &http.Client{}
	response, err := client.Do(request)
	if err != nil {
		panic(err)
	}
	defer response.Body.Close()

	body, _ := ioutil.ReadAll(response.Body)
	//fmt.Println(string(body))
	statusCode := response.StatusCode
	if statusCode != 200 {
		//fmt.Println("Process failed，HTTP StatusCode: " + strconv.Itoa(statusCode))
		return fmt.Sprintf("HTTP StatusCode:", strconv.Itoa(statusCode)), fmt.Errorf("Process failed")
	}

	var resultMap map[string]interface{}
	err = json.Unmarshal([]byte(body), &resultMap)
	if err != nil {
		panic(err)
	}
	var result string = resultMap["result"].(string)

	return result, nil
}

func CallHandle(w http.ResponseWriter, r *http.Request) {
	switch r.Method {
	case "POST":
		var content Content
		var format string = "pcm"
		var sampleRate int = 8000
		var enablePunctuationPrediction bool = true
		var enableInverseTextNormalization bool = true
		var enableVoiceDetection = false

		/* Parsed parameters from http request */
		body, _ := ioutil.ReadAll(r.Body)

		header := r.Header.Get("Content-Type")
		fmt.Println("Content-Type ", header)

		mediaType, params, err := mime.ParseMediaType(header)
		if err != nil {
			log.Fatal(err)
		}
		if strings.HasPrefix(mediaType, "audio/") {
			//sampleRate = params["samplerate"]
			fmt.Println("Params ", params["samplerate"])
			fmt.Println("media type ", mediaType)
			if mediaType == "audio/pcm" {
				format = "pcm"
			}
		}

		/* second time to send http request */
		result, err := OriginateProcess(appkey, token, body, format, sampleRate,
			enablePunctuationPrediction, enableInverseTextNormalization, enableVoiceDetection)
		if err != nil {
			http.Error(w, err.Error(), 500)
		}
		fmt.Println("Auto Speech Recongition result ", result)

		status, err := DetectCurrentStatus(result)
		if err != nil {
			http.Error(w, err.Error(), 500)
		}

		if status == "Unknown" {
			content.Code = "200"
			content.IsVacantNumber = 0
			content.HangupState = "909"
			content.VacantNumberType = "ttt"
		} else {
			content.Code = "200"
			content.IsVacantNumber = 1
			content.HangupState = "910"
			content.VacantNumberType = status
		}

		bs, err := json.Marshal(content)
		if err != nil {
			fmt.Println(err)
		} else {
			fmt.Fprint(w, string(bs))
		}

	default:
		http.Error(w, http.StatusText(405), 405)
	}
}


func process(appkey string, token string, audio []byte, format string, sampleRate string,
	enablePunctuationPrediction bool, enableInverseTextNormalization bool, enableVoiceDetection bool) (string, error) {
	var url string = "http://nls-gateway.cn-shanghai.aliyuncs.com/stream/v1/asr"
	url = url + "?appkey=" + appkey
	url = url + "&format=" + format
	url = url + "&sample_rate=" + sampleRate
	if enablePunctuationPrediction {
		url = url + "&enable_punctuation_prediction=" + "true"
	}
	if enableInverseTextNormalization {
		url = url + "&enable_inverse_text_normalization=" + "true"
	}
	if enableVoiceDetection {
		url = url + "&enable_voice_detection=" + "false"
	}
	request, err := http.NewRequest("POST", url, bytes.NewBuffer(audio))
	if err != nil {
		panic(err)
	}
	request.Header.Add("X-NLS-Token", token)
	request.Header.Add("Content-Type", "application/octet-stream")

	client := &http.Client{}
	response, err := client.Do(request)
	if err != nil {
		panic(err)
	}
	defer response.Body.Close()

	body, _ := ioutil.ReadAll(response.Body)
	//fmt.Println(string(body))
	statusCode := response.StatusCode
	if statusCode != 200 {
		//fmt.Println("Process failed，HTTP StatusCode: " + strconv.Itoa(statusCode))
		return fmt.Sprintf("HTTP StatusCode:", strconv.Itoa(statusCode)), fmt.Errorf("Process failed")
	}

	var resultMap map[string]interface{}
	err = json.Unmarshal([]byte(body), &resultMap)
	if err != nil {
		panic(err)
	}
	var result string = resultMap["result"].(string)

	return result, nil
}


func DetectCurrentStatus(result string) (string, error) {
	//var tag string
	var status string
	if result == "" {
		return "", fmt.Errorf("Result is null")
	}

	if strings.Contains(result, "无人接听") {
		//tag = "Noanswer"
		status = "910"
	} else if strings.Contains(result, "暂时无法接通") {
		//tag = "Unconnect"
		status = "911"
	} else if strings.Contains(result, "正在通话") {
		//tag = "Busy"
		status = "912"
	} else if strings.Contains(result, "停机") {
		//tag = "OutOfService"
		status = "913"
	} else if strings.Contains(result, "关机") {
		//tag = "Poweroff"
		status = "914"
	} else if strings.Contains(result, "空号") {
		//tag = "Vacant"
		status = "915"
	} else if strings.Contains(result, "正忙") {
		//tag = "Reject"
		status = "916"
	} else if strings.Contains(result, "无法接听") {
		//tag = "Disconnect"
		status = "917"
	} else {
		//tag = "Unknown"
		status = "933"
	}

	return status, nil
}


func VacantHandler(w http.ResponseWriter, r *http.Request) {
	switch r.Method {
	case "POST":
		var content Content
		var format string = "pcm"
		var sampleRate string
		var enablePunctuationPrediction bool = true
		var enableInverseTextNormalization bool = true
		var enableVoiceDetection = false
		var httpResult Result

		/* Parse parameters from http post request */
		body, _ := ioutil.ReadAll(r.Body)
		header := r.Header.Get("Content-Type")
		mediaType, params, err := mime.ParseMediaType(header)
		if err != nil {
			log.Fatal(err)
		}
		if strings.HasPrefix(mediaType, "audio/") {
			sampleRate = params["samplerate"]
			if mediaType == "audio/pcm" {
				format = "pcm"
			}
		}
		fmt.Println("Content header " + header)

		/* Generate aliyun asr application access token */
		client, err := sdk.NewClientWithAccessKey("cn-shanghai",
				"LTAIjnlQdNmuZxfx", "0SWtcChhOrE6puneote4erXr7jejuJ")
		if err != nil {
			panic(err)
		}
		request := requests.NewCommonRequest()
		request.Method = "POST"
		request.Domain = "nls-meta.cn-shanghai.aliyuncs.com"
		request.ApiName = "CreateToken"
		request.Version = "2019-02-28"
		response, err := client.ProcessCommonRequest(request)
		if err != nil {
			panic(err)
		}

		if response.GetHttpStatus() != 200 {
			http.Error(w, "get_token_error", 500)
		}

		err = json.Unmarshal([]byte(response.GetHttpContentString()), &httpResult)
		if err != nil {
			panic(err)
		}
		//fmt.Printf("%d\n%s\n", httpResult.Token.ExpireTime, httpResult.Token.Id)

		/* aliyun asr process, get asr result */
		result, err := process(appkey, httpResult.Token.Id, body, format, sampleRate,
			enablePunctuationPrediction, enableInverseTextNormalization, enableVoiceDetection)
		if err != nil {
			http.Error(w, err.Error(), 500)
		}
		fmt.Println("Ringing result ", result)

		status, err := DetectCurrentStatus(result)
		if err != nil {
			http.Error(w, err.Error(), 500)
		}

		if status == "933" {
			content.Code = "200"
			content.IsVacantNumber = 0
			content.HangupState = "900"
			content.VacantNumberType = "none"
		} else {
			content.Code = "200"
			content.IsVacantNumber = 1
			content.HangupState = status
			content.VacantNumberType = status
		}

		jsonString, err := json.Marshal(content)
		if err != nil {
			fmt.Println(err)
			http.Error(w, err.Error(), 405)
		}

		fmt.Fprint(w, string(jsonString))

	default:
		http.Error(w, http.StatusText(405), 405)
	}
}


func main() {
	http.HandleFunc("/vacantNumber", VacantHandler)
	http.HandleFunc("/rojita/call", CallHandle)
	err := http.ListenAndServe(":4000", nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err)
	}
}
