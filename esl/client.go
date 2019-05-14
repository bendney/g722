// Copyright 2013 Alexandre Fiori
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Event Socket client that connects to FreeSWITCH to originate a new call.
package main

import (
	"encoding/json"
	"fmt"
	"log"
	"github.com/fiorix/go-eventsocket/eventsocket"
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

	const originator_uuid string = "2242dee0-35b6-11e9-b900-355947430f71"
	const phone_number string = "18625221611"
	const sip_server string = "10.10.5.250"

	client.Send("events json CHANNEL_CREATE CHANNEL_DESTROY CHANNEL_ANSWER CHANNEL_HANGUP CHANNEL_CALLSTATE CUSTOM robot::report")

	var profile = Profile{"1.0", "6757", "10050", "10", "a-f6f0cf6fe5c9479d876c9eae61857ffa", "2", "91008", "keda", "template_001", "1", "/tmp/log/a-f6f0cf6fe5c9479d876c9eae61857ffa", "1", "10.200.1.37:18086", "10.100.2.38:4000"}
	json, err := json.Marshal(profile)
	if err != nil {
		return
	}
	json_byte := fmt.Sprintf("`%s`", json)

	rawIn, err := json.Marshal(json_byte)
	if err != nil {
		return
	}

	command := fmt.Sprintf("bgapi originate {origination_caller_id_number=853692}sofia/external/%s@%s &bridge({origination_uuid=%s,sip_status=183,robot_input=%s}Robot)\n",
	phone_number, sip_server, originator_uuid, string(rawIn))
	fmt.Println(command)

	client.Send(command)

	for {
		event, err := client.ReadEvent()
		if err != nil {
			log.Fatal(err)
		}

		//fmt.Println("Received Event", event.Get("Event-Name"))

		if event.Get("Event-Name") == "CHANNEL_ANSWER" {
			if event.Get("Other-Type") == "originatee" {
				client.Send(fmt.Sprintf("bgapi uuid_send_message %s sip_status", event.Get("Other-Leg-Unique-Id")))
			}
		}

		if event.Get("Event-Name") == "CUSTOM" {
			fmt.Println("Event-Subclass", event.Get("Event-Subclass"))
			fmt.Println("Variable_uuid", event.Get("Variable_uuid"))
			fmt.Println("Event-Info", event.Get("Event_Info"))
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


