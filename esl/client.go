// Copyright 2013 Alexandre Fiori
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Event Socket client that connects to FreeSWITCH to originate a new call.
package main

import (
	"encoding/json"
	"fmt"
	"github.com/fiorix/go-eventsocket/eventsocket"
	"log"
)

type Profile struct {
	Version          string `json:"version"`
	Sequence         string `json:"sequence"`
	PlanId           string `json:"plan_id"`
	RobotIndex       string `json:"robot_index"`
	CallId           string `json:"call_id"`
	CallerId         string `json:"caller_id"`
	Callee           string `json:"callee"`
	AsrEngine        string `json:"asr_engine"`
	TemplateId       string `json:"template_id"`
	AudioRecord      string `json:"audio_record"`
	Record_path      string `json:"record_path"`
	EarlyMediaRecord string `json:"early_media_record"`
	NlpServer        string `json:"nlp_server"`
	AsrServer        string `json:"asr_server"`
}

func main() {
	client, err := eventsocket.Dial("127.0.0.1:8021", "ClueCon")
	if err != nil {
		log.Fatal(err)
	}

	const originator_uuid = "2242dee0-35b6-11e9-b900-355947430f71"
	var phone string = "18625221611"
	var server string = "10.10.5.250"

	client.Send("events json CHANNEL_CREATE CHANNEL_DESTROY CHANNEL_ANSWER CHANNEL_HANGUP CHANNEL_CALLSTATE CUSTOM robot::report")

	var profile = Profile{"1.0", "6757", "10050", "10", "a-3892e9971c5d4b378b4fe17e9770d4d8", "2", "91008", "keda", "template_001", "1", "/tmp/log/a-3892e9971c5d4b378b4fe17e9770d4d8", "1", "10.200.1.37:18086", "10.20.4.139:4000"}
	js, err := json.Marshal(profile)
	if err != nil {
		return
	}
	fmt.Printf("Robot input parameter %s\n", js)

	jsondata := []byte(`{"version":"1.0"\,"sequence":"6757"\,"plan_id":"10050"\,"robot_index":"10"\,"call_id":"a-f6f0cf6fe5c9479d876c9eae61857ffa"\,"caller_id":"2"\,"callee":"91008"\,"asr_engine":"keda"\,"template_id":"template_001"\,"audio_record":"1"\,"record_path":"/tmp/log/a-f6f0cf6fe5c9479d876c9eae61857ffa"\,"early_media_record":"1"\,"nlp_server":"10.20.4.218:8080"\,"asr_server":"10.100.2.38:4000"}`)

	command := fmt.Sprintf("bgapi originate {origination_caller_id_number=853692}sofia/external/%s@%s &bridge({origination_uuid=%s,sip_status=183,robot_input=%s}Robot)\n", phone, server, originator_uuid, jsondata)
	fmt.Println(command)

	client.Send(command)

	for {
		ev, err := client.ReadEvent()
		if err != nil {
			log.Fatal(err)
		}

		fmt.Println("Received Event", ev.Get("Event-Name"))

		if ev.Get("Event-Name") == "CHANNEL_ANSWER" {
			if ev.Get("Other-Type") == "originatee" {
				client.Send(fmt.Sprintf("bgapi uuid_send_message %s sip_status", ev.Get("Other-Leg-Unique-Id")))
			}
		}
		if ev.Get("Event-Name") == "CUSTOM" {
			fmt.Println("variable uuid", ev.Get("Variable_uuid"))
			fmt.Println("subclass", ev.Get("Event-Subclass"))
			fmt.Println("Event-Info", ev.Get("Event-Info"))
			//ev.PrettyPrint()
		}

		/*
			if ev.Get("Answer-State") == "hangup" {
				ev.PrettyPrint()
				break
			}
		*/
	}
	client.Close()
}
